/*
 *  qsi_myspin.c
 *  Copyright © 2015 QSI Inc.
 *  All rights reserved.
 *  
 *       Author : Lester Yang <lester.yang@qsitw.com>
 *  Description : mySPIN driver
 */
 
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/kref.h>
#include <linux/uaccess.h>
#include <linux/usb.h>
#include <linux/usb/ch9.h>
#include <linux/mutex.h>
#include "../../hid/hid-ids.h"

/*
* Following macro infomation is from linux/usb.h, linux/usb/ch9.h, linux/usb/audio.h, and linux/hid.h
* ==========================================================================
 * Descriptor types
 * #define USB_DT_DEVICE          0x01
 * #define USB_DT_CONFIG          0x02
 * #define USB_DT_STRING          0x03
 * #define USB_DT_INTERFACE               0x04
 * #define USB_DT_ENDPOINT                 0x05
 * 
 * Device and/or Interface Class codes
 * #define USB_CLASS_PER_INTERFACE  0     
 * #define USB_CLASS_AUDIO               1
 * #define USB_CLASS_COMM               2
 * #define USB_CLASS_HID                   3
 * #define USB_CLASS_PHYSICAL           5
 * #define USB_CLASS_STILL_IMAGE      6
 * #define USB_CLASS_PRINTER             7
 * #define USB_CLASS_MASS_STORAGE  8
 * #define USB_CLASS_APP_SPEC          0xfe
 * #define USB_CLASS_VENDOR_SPEC    0xff
 * 
 * USB directions
 * #define USB_DIR_OUT                          0               //to device
 * #define USB_DIR_IN                           0x80            //to host 
 * 
 * USB types, the second of three bRequestType fields
 * #define USB_TYPE_STANDARD             (0x00 << 5)
 * #define USB_TYPE_CLASS                   (0x01 << 5)
 * #define USB_TYPE_VENDOR                 (0x02 << 5)
 * 
 * USB recipients, the third of three bRequestType fields
 * #define USB_RECIP_DEVICE                0x00
 * #define USB_RECIP_INTERFACE           0x01
 * #define USB_RECIP_ENDPOINT            0x02
 * 
 * Standard requests, for the bRequest field of a SETUP packet.
 * #define USB_REQ_GET_STATUS              0x00
 * #define USB_REQ_CLEAR_FEATURE           0x01
 * #define USB_REQ_SET_FEATURE              0x03
 * #define USB_REQ_SET_ADDRESS             0x05
 * #define USB_REQ_GET_DESCRIPTOR         0x06
 * #define USB_REQ_SET_DESCRIPTOR         0x07
 * #define USB_REQ_GET_CONFIGURATION    0x08
 * #define USB_REQ_SET_CONFIGURATION    0x09
 * #define USB_REQ_GET_INTERFACE           0x0A
 * #define USB_REQ_SET_INTERFACE           0x0B
 * #define USB_REQ_SYNCH_FRAME             0x0C
 *
 * USB HID (Human Interface Device) interface class code
 * #define USB_INTERFACE_CLASS_HID         3
 *
 * USB HID interface subclass and protocol codes
 * #define USB_INTERFACE_SUBCLASS_BOOT        1
 * #define USB_INTERFACE_PROTOCOL_KEYBOARD 1
 * #define USB_INTERFACE_PROTOCOL_MOUSE      2
 *
 * HID class requests
 * #define HID_REQ_GET_REPORT            0x01
 * #define HID_REQ_GET_IDLE                0x02
 * #define HID_REQ_GET_PROTOCOL        0x03
 * #define HID_REQ_SET_REPORT            0x09
 * #define HID_REQ_SET_IDLE                0x0A
 * #define HID_REQ_SET_PROTOCOL        0x0B
 *
 * HID class descriptor types
 * #define HID_DT_HID                      (USB_TYPE_CLASS | 0x01)
 * #define HID_DT_REPORT                 (USB_TYPE_CLASS | 0x02)
 * #define HID_DT_PHYSICAL              (USB_TYPE_CLASS | 0x03)
 * 
 * HID report types
 * #define HID_INPUT_REPORT          0
 * #define HID_OUTPUT_REPORT       1
 * #define HID_FEATURE_REPORT      2

 * Audio Interface Subclass Codes
 * #define USB_SUBCLASS_AUDIOCONTROL      0x01
 * #define USB_SUBCLASS_AUDIOSTREAMING    0x02
 * #define USB_SUBCLASS_MIDISTREAMING      0x03
 * ==========================================================================
 */
#define CTRL_OUT (USB_TYPE_VENDOR | USB_DIR_OUT)
#define CTRL_IN (USB_TYPE_VENDOR | USB_DIR_IN)
#define AOA_CTRL_OUT (CTRL_OUT | USB_RECIP_DEVICE)
#define AOA_CTRL_IN (CTRL_IN | USB_RECIP_DEVICE)


#define AOA_REQ_PROTOCOL            (51)
#define AOA_REQ_SETPROTO            (52)
#define AOA_PROTO_MANUFACTURE_INDEX	(0)
#define AOA_PROTO_MODEL_INDEX 		(1)
#define AOA_PROTO_DESCRIPTION_INDEX	(2)
#define AOA_PROTO_VERSION_INDEX		(3)
#define AOA_PROTO_URI_INDEX			(4)
#define AOA_PROTO_SERIAL_INDEX		(5)
#define AOA_REQ_ACCESSORY		    (53)
#define AOA_REQ_REGISTER_HID		(54)
#define AOA_REQ_UNREGISTER_HID		(55)
#define AOA_REQ_SET_HID_REPORT	    (56)
#define AOA_SEND_HID_EVENT		    (57)
#define AOA_REQ_AUDIO		        (58)
#define AOA_PID_BASE				(0x2d00) /* accessory */
#define AOA_PID_WITH_ADB		    (0x2d01) /* accessory + adb */
#define AOA_PID_AUDIO_ONLY          (0x2d02) /* audio */
#define AOA_PID_AUDIO_WITH_ADB		(0x2d03) /* audio + adb */
#define AOA_PID_WITH_AUDIO		    (0x2d04) /* accessory + audio */
#define AOA_PIO_WITH_AUDIO_ADB      (0x2d05) /* accessory + audio + adb */

#define ADK2012_MANUFACTURE_STRING			("BSOT")
#define ADK2012_MODEL_STRING				("accessory and audio")
#define ADK2012_DESCRIPTION_STRING			("Qsi mySpin")
#define ADK2012_VERSION_STRING				("2.0")
#define ADK2012_URI_STRING				    ("com.bosch.myspin.launcherapp")
#define ADK2012_SERIAL_STRING				("0000000012345678")

#define USB_VENDOR_ID_SONY_ERICSSON (0x0fce)
#define USB_VENDOR_ID_GOOGLE        (0x18d1)


static const struct usb_device_id qsi_myspin_table[] = {
    { .match_flags = USB_DEVICE_ID_MATCH_INT_CLASS | USB_DEVICE_ID_MATCH_INT_SUBCLASS | USB_DEVICE_ID_MATCH_INT_PROTOCOL,
      .bInterfaceClass = USB_CLASS_VENDOR_SPEC,
      .bInterfaceSubClass = USB_SUBCLASS_VENDOR_SPEC,
      .bInterfaceProtocol = 0 
    },
	{ }
};
MODULE_DEVICE_TABLE(usb, qsi_myspin_table);

/* Get a minor range for your devices from the usb maintainer */
#define USB_QSI_MYSPIN_MINOR_BASE	192
/* our private defines. if this grows any larger, use your own .h file */
#define MAX_TRANSFER		(PAGE_SIZE - 512)
/* MAX_TRANSFER is chosen so that the VM is not stressed by
   allocations > PAGE_SIZE and the number of packets in a page
   is an integer 512 is the largest possible packet on EHCI */
#define WRITES_IN_FLIGHT	8

/* Structure to hold all of our device specific stuff */
struct usb_qsi_myspin {
	struct usb_device	    *udev;			        /* the usb device for this device */
	struct usb_interface	*interface;		        /* the interface for this device */
	struct semaphore	    limit_sem;		        /* limiting the number of writes in progress */
	struct usb_anchor	    submitted;		        /* in case we need to retract our submissions */

    struct urb		        *bulk_in_urb;		    /* the urb to read data with */
	unsigned char           *bulk_in_buffer;	    /* the buffer to receive data */
	size_t			        bulk_in_size;		    /* the size of the receive buffer */
	size_t			        bulk_in_filled;		    /* number of bytes in the buffer */
	size_t			        bulk_in_copied;		    /* already copied to user space */
	__u8			        bulk_in_endpointAddr;	/* the address of the bulk in endpoint */
	__u8			        bulk_out_endpointAddr;  /* the address of the bulk out endpoint */

    unsigned char           *ctrl_buffer;
    size_t					ctrl_size;
    size_t					ctrl_filled;
    size_t					ctrl_copied;
    __u8					ctrl_epAddr;
    
	int			            errors;			        /* the last request tanked */
	bool			        ongoing_read;	        /* a read is going on */
	bool			        processed_urb;		    /* indicates we haven't processed the urb */
	spinlock_t		        err_lock;		        /* lock for errors */
	struct kref		        kref;
	struct mutex		    io_mutex;		        /* synchronize I/O with disconnect */
    wait_queue_head_t       bulk_in_wait;           /* to wait for an ongoing read */
};

#define to_qsi_myspin_dev(d) container_of(d, struct usb_qsi_myspin, kref)

static struct usb_driver qsi_myspin_driver;
static void qsi_myspin_draw_down(struct usb_qsi_myspin *dev);

static void qsi_myspin_delete(struct kref *kref)
{
	struct usb_qsi_myspin *dev = to_qsi_myspin_dev(kref);

	if(dev->bulk_in_urb) usb_free_urb(dev->bulk_in_urb);
    if(dev->bulk_in_buffer) kfree(dev->bulk_in_buffer);
    if(dev->ctrl_buffer) kfree(dev->ctrl_buffer);
	usb_put_dev(dev->udev);
	kfree(dev);
}

static int qsi_myspin_open(struct inode *inode, struct file *file)
{
    struct usb_qsi_myspin *dev;
    struct usb_interface *interface;
    int subminor;
    int retval = 0;

    subminor = iminor(inode);

    interface = usb_find_interface(&qsi_myspin_driver, subminor);
    if (!interface) 
    {
        err("%s - error, can't find device for minor %d", __func__, subminor);
        retval = -ENODEV;
        goto exit;
    }

    dev = usb_get_intfdata(interface);
    if (!dev)
    {
        retval = -ENODEV;
        goto exit;
    }

    retval = usb_autopm_get_interface(interface);
    if(retval)
        goto exit;

    /* increment our usage count for the device */
    kref_get(&dev->kref);

    /* save our object in the file's private structure */
    file->private_data = dev;

exit:
    return retval;
}

static int qsi_myspin_release(struct inode *inode, struct file *file)
{
	struct usb_qsi_myspin *dev;

	dev = file->private_data;
	if (dev == NULL)
		return -ENODEV;

	/* allow the device to be autosuspended */
	mutex_lock(&dev->io_mutex);
	if (dev->interface)
		usb_autopm_put_interface(dev->interface);
	mutex_unlock(&dev->io_mutex);

	/* decrement the count on our device */
	kref_put(&dev->kref, qsi_myspin_delete);
	return 0;
}

static int qsi_myspin_flush(struct file *file, fl_owner_t id)
{
	struct usb_qsi_myspin *dev;
	int res;

	dev = file->private_data;
	if (dev == NULL)
		return -ENODEV;

	/* wait for io to stop */
	mutex_lock(&dev->io_mutex);
	qsi_myspin_draw_down(dev);

	/* read out errors, leave subsequent opens a clean slate */
	spin_lock_irq(&dev->err_lock);
	res = dev->errors ? (dev->errors == -EPIPE ? -EPIPE : -EIO) : 0;
	dev->errors = 0;
	spin_unlock_irq(&dev->err_lock);

	mutex_unlock(&dev->io_mutex);

	return res;
}

static void  qsi_myspin_read_bulk_callback(struct urb *urb)
{
	struct usb_qsi_myspin *dev;

	dev = urb->context;

	spin_lock(&dev->err_lock);
	/* sync/async unlink faults aren't errors */
	if (urb->status) {
		if (!(  urb->status == -ENOENT      ||
		        urb->status == -ECONNRESET  ||
		        urb->status == -ESHUTDOWN))
        {      
			err("%s - nonzero write bulk status received: %d", __func__, urb->status);
        }
		dev->errors = urb->status;
	} else 
	{
		dev->bulk_in_filled = urb->actual_length;
	}
	dev->ongoing_read = 0;
	spin_unlock(&dev->err_lock);

	wake_up_interruptible(&dev->bulk_in_wait);
}

static int  qsi_myspin_read_io(struct usb_qsi_myspin *dev, size_t count)
{
	int rv;

	/* prepare a read */
	usb_fill_bulk_urb(  dev->bulk_in_urb,
			            dev->udev,
			            usb_rcvbulkpipe(dev->udev, dev->bulk_in_endpointAddr),
			            dev->bulk_in_buffer,
			            min(dev->bulk_in_size, count),
			            qsi_myspin_read_bulk_callback,
			            dev);
    
	/* tell everybody to leave the URB alone */
	spin_lock_irq(&dev->err_lock);
	dev->ongoing_read = 1;
	spin_unlock_irq(&dev->err_lock);

    /* submit bulk in urb, which means no data to deliver */
    dev->bulk_in_filled = 0;
    dev->bulk_in_copied = 0;
    
	/* do it */
	rv = usb_submit_urb(dev->bulk_in_urb, GFP_KERNEL);
	if (rv < 0) 
    {
		err("%s - failed submitting read urb, error %d", __func__, rv);
		dev->bulk_in_filled = 0;
		rv = (rv == -ENOMEM) ? rv : -EIO;
		spin_lock_irq(&dev->err_lock);
		dev->ongoing_read = 0;
		spin_unlock_irq(&dev->err_lock);
	}
	return rv;
}

static ssize_t qsi_myspin_read(struct file *file, char *buffer, size_t count, loff_t *ppos)
{
    struct usb_qsi_myspin *dev;
    int rv;
    bool ongoing_io;

    dev = file->private_data;

    /* if we cannot read at all, return EOF */
    if (!dev->bulk_in_urb || !count)
        return 0;

    /* no concurrent readers */
    rv = mutex_lock_interruptible(&dev->io_mutex);
    if (rv < 0)
        return rv;

    if (!dev->interface) 
    {   /* disconnect() was called */
        rv = -ENODEV;
        goto exit;
    }

    /* if IO is under way, we must not touch things */
retry:
    spin_lock_irq(&dev->err_lock);
    ongoing_io = dev->ongoing_read;
    spin_unlock_irq(&dev->err_lock);

    if (ongoing_io) 
    {
        /* nonblocking IO shall not wait */
        if (file->f_flags & O_NONBLOCK)
        {
            rv = -EAGAIN;
            goto exit;
        }
        /*
              * IO may take forever
              * hence wait in an interruptible state
              */
        rv = wait_event_interruptible(dev->bulk_in_wait, (!dev->ongoing_read));
        if (rv < 0)
            goto exit;
        /*
              * by waiting we also semiprocessed the urb
              * we must finish now
              */
        dev->bulk_in_copied = 0;
    }

    /* errors must be reported */
    rv = dev->errors;
    
    if (rv < 0) 
    {   /* any error is reported once */
        dev->errors = 0;
        /* to preserve notifications about reset */
        rv = (rv == -EPIPE) ? rv : -EIO;
        /* no data to deliver */
        dev->bulk_in_filled = 0;
        /* report it */
        goto exit;
    }

    /*
        * if the buffer is filled we may satisfy the read
        * else we need to start IO
        */

    if (dev->bulk_in_filled) 
    {   /* we had read data */
        size_t available = dev->bulk_in_filled - dev->bulk_in_copied;
        size_t chunk = min(available, count);

        if (!available) 
        {   /*
                     * all data has been used
                     * actual IO needs to be done
                     */
            rv = qsi_myspin_read_io(dev, count);
            if (rv < 0)
                goto exit;
            else
                goto retry;
        }
        /*
              * data is available
              * chunk tells us how much shall be copied
              */
        if (copy_to_user(buffer, dev->bulk_in_buffer + dev->bulk_in_copied, chunk))
            rv = -EFAULT;
        else
            rv = chunk;

        dev->bulk_in_copied += chunk;

        /*
              * if we are asked for more than we have,
              * we start IO but don't wait
              */
        if (available < count)
            qsi_myspin_read_io(dev, count - chunk);
    } 
    else
    {   /* no data in the buffer */
        rv = qsi_myspin_read_io(dev, count);
        if (rv < 0)
            goto exit;
        else if (!(file->f_flags & O_NONBLOCK))
            goto retry;
        rv = -EAGAIN;
    }
exit:
    mutex_unlock(&dev->io_mutex);
    return rv;
}


static void qsi_myspin_write_bulk_callback(struct urb *urb)
{
	struct usb_qsi_myspin *dev;

	dev = urb->context;

	/* sync/async unlink faults aren't errors */
	if (urb->status) {
		if (!(  urb->status == -ENOENT      ||
		        urb->status == -ECONNRESET  ||
		        urb->status == -ESHUTDOWN))
        {      
			err("%s - nonzero write bulk status received: %d", __func__, urb->status);
        }
		spin_lock(&dev->err_lock);
		dev->errors = urb->status;
		spin_unlock(&dev->err_lock);
	}

	/* free up our allocated buffer */
	usb_free_coherent(urb->dev, urb->transfer_buffer_length, urb->transfer_buffer, urb->transfer_dma);
	up(&dev->limit_sem);
}


static ssize_t qsi_myspin_write(struct file *file, const char *user_buffer, size_t count, loff_t *ppos)
{
    struct usb_qsi_myspin *dev;
    int retval = 0;
    struct urb *urb = NULL;
    char *buf = NULL;
    size_t writesize = min(count, (size_t)MAX_TRANSFER);

    dev = file->private_data;

    /* verify that we actually have some data to write */
    if (count == 0)
        goto exit;

    /*
        * limit the number of URBs in flight to stop a user from using up all
        * RAM
        */
    if (!(file->f_flags & O_NONBLOCK))
    {
        if (down_interruptible(&dev->limit_sem)) 
        {
            retval = -ERESTARTSYS;
            goto exit;
        }
    } 
    else 
    {
        if (down_trylock(&dev->limit_sem)) 
        {
            retval = -EAGAIN;
            goto exit;
        }
    }

    spin_lock_irq(&dev->err_lock);
    retval = dev->errors;
    if (retval < 0) 
    {
        /* any error is reported once */
        dev->errors = 0;
        /* to preserve notifications about reset */
        retval = (retval == -EPIPE) ? retval : -EIO;
    }
    spin_unlock_irq(&dev->err_lock);
    if (retval < 0)
        goto error;

    /* create a urb, and a buffer for it, and copy the data to the urb */
    urb = usb_alloc_urb(0, GFP_KERNEL);
    if (!urb) 
    {
        retval = -ENOMEM;
        goto error;
    }

    buf = usb_alloc_coherent(dev->udev, writesize, GFP_KERNEL, &urb->transfer_dma);
    if (!buf)
    {
        retval = -ENOMEM;
        goto error;
    }
    if (copy_from_user(buf, user_buffer, writesize))
    {
        retval = -EFAULT;
        goto error;
    }

    /* this lock makes sure we don't submit URBs to gone devices */
    mutex_lock(&dev->io_mutex);
    if (!dev->interface) 
    {   /* disconnect() was called */
        mutex_unlock(&dev->io_mutex);
        retval = -ENODEV;
        goto error;
    }

    /* initialize the urb properly */
    usb_fill_bulk_urb(  urb, 
                        dev->udev,
                        usb_sndbulkpipe(dev->udev, dev->bulk_out_endpointAddr),
                        buf, 
                        writesize, 
                        qsi_myspin_write_bulk_callback, 
                        dev);
    urb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;
    usb_anchor_urb(urb, &dev->submitted);

    /* send the data out the bulk port */
    retval = usb_submit_urb(urb, GFP_KERNEL);
    mutex_unlock(&dev->io_mutex);
    if (retval) 
    {
        err("%s - failed submitting write urb, error %d", __func__, retval);
        goto error_unanchor;
    }

    /*
        * release our reference to this urb, the USB core will eventually free
        * it entirely
        */
    usb_free_urb(urb);

    return writesize;

error_unanchor:
    usb_unanchor_urb(urb);
error:
    if (urb) 
    {
        usb_free_coherent(dev->udev, writesize, buf, urb->transfer_dma);
        usb_free_urb(urb);
    }
    up(&dev->limit_sem);

exit:
    return retval;

}


static const struct file_operations qsi_myspin_fops = {
	.owner =	THIS_MODULE,
	.read =		qsi_myspin_read,
	.write =	qsi_myspin_write,
	.open =		qsi_myspin_open,
	.release =	qsi_myspin_release,
	.flush =	qsi_myspin_flush,
	.llseek =	noop_llseek,
};

static struct usb_class_driver  qsi_myspin_class = {
	.name =		"qsi_myspin%d",
	.fops =		&qsi_myspin_fops,
	.minor_base =	USB_QSI_MYSPIN_MINOR_BASE,
};


static void qsi_myspin_set_proto(struct usb_qsi_myspin *dev, int idx, const char* str)
{
    int res;

    dev->ctrl_size = strlen(str) + 1;
    memcpy(dev->ctrl_buffer, str, dev->ctrl_size);
    
    res = usb_control_msg ( dev->udev,
                            usb_sndctrlpipe(dev->udev, dev->ctrl_epAddr),
                            AOA_REQ_SETPROTO,
                            AOA_CTRL_OUT,
                            0,
                            idx,
                            dev->ctrl_buffer,
                            dev->ctrl_size,
                            500);

    if(res < 0)
        printk("set protocol error!!\n");
    else
        printk("set protocol : %s\n", str);

}

static int qsi_myspin_set_audio_mode(struct usb_qsi_myspin *dev, char mode)
{
    return usb_control_msg ( dev->udev,
                            usb_sndctrlpipe(dev->udev, dev->ctrl_epAddr),
                            AOA_REQ_AUDIO,
                            AOA_CTRL_OUT,
                            mode,
                            0,
                            dev->ctrl_buffer,
                            0,
                            500);
}

static int qsi_myspin_switch_to_accessory_mode(struct usb_qsi_myspin *dev)
{
    return usb_control_msg ( dev->udev,
                            usb_sndctrlpipe(dev->udev, dev->ctrl_epAddr),
                            AOA_REQ_ACCESSORY,
                            AOA_CTRL_OUT,
                            0,
                            0,
                            dev->ctrl_buffer,
                            0,
                            500);
}

static int qsi_myspin_probe(struct usb_interface *interface, const struct usb_device_id *id)
{
    struct usb_qsi_myspin           *dev = NULL;
    struct usb_host_interface       *host_intf;
    struct usb_config_descriptor    *cfg_desc;
    struct usb_interface_descriptor *intf_desc;
    struct usb_endpoint_descriptor  *endpoint_desc;

    int i, res;
    char proto[2]={0};
    size_t buffer_size;

    printk("kernel : qsi_myspin_probe !!\n");

	dev = kzalloc(sizeof(*dev), GFP_KERNEL);

	if (!dev) {
		printk("kernel : Out of memory\n");
		goto error;
	}

    dev->bulk_in_buffer = NULL;
    dev->ctrl_buffer = NULL;
    dev->bulk_in_urb = NULL;

	kref_init(&dev->kref);
	sema_init(&dev->limit_sem, WRITES_IN_FLIGHT);
	mutex_init(&dev->io_mutex);
	spin_lock_init(&dev->err_lock);
	init_usb_anchor(&dev->submitted);
    init_waitqueue_head(&dev->bulk_in_wait);

    dev->udev        = usb_get_dev(interface_to_usbdev(interface));
    dev->interface   = interface;
    dev->ctrl_epAddr = 0x0;
    dev->ctrl_buffer = kzalloc(2048, GFP_KERNEL);

    host_intf = dev->interface->cur_altsetting;
    cfg_desc = &dev->udev->actconfig->desc;
    intf_desc = &host_intf->desc;
    
#if 0
    printk(KERN_EMERG "Device, VID =%d\r\n",id->idVendor);
    printk(KERN_EMERG "Device, PID =%d\r\n",id->idProduct);
    printk(KERN_EMERG "Device, Class =%d/%d/%d\r\n",id->bInterfaceClass,id->bInterfaceSubClass,id->bInterfaceProtocol);
    printk(KERN_EMERG "Device, Config Num=%d\r\n",(interface_to_usbdev(interface)->descriptor).bNumConfigurations);
    printk(KERN_EMERG " Config, ConfigID =%d\r\n",cfg_desc->bConfigurationValue);
    printk(KERN_EMERG " Config, Total Interface =%d\r\n",cfg_desc->bNumInterfaces);
    printk(KERN_EMERG " Config, MaxPower =%d\r\n",cfg_desc->bMaxPower);
    printk(KERN_EMERG "     Inter, InterfaceID =%d\r\n",intf_desc->bInterfaceNumber);
    printk(KERN_EMERG "     Inter, Total EndPoint =%d\r\n",intf_desc->bNumEndpoints);
    printk(KERN_EMERG "     Inter, Class =%d/%d/%d\r\n",intf_desc->bInterfaceClass,intf_desc->bInterfaceSubClass,intf_desc->bInterfaceProtocol);

	for (i = 0; i < host_intf->desc.bNumEndpoints; ++i) 
    {
        endpoint_desc = &host_intf->endpoint[i].desc;
		printk(KERN_EMERG "		EndpointNum: %d\r\n",i);
    	printk(KERN_EMERG "         Endpoint, bLength=%d\r\n",endpoint_desc->bLength);
   	 	printk(KERN_EMERG "         Endpoint, bDescriptorType=%d\r\n",endpoint_desc->bDescriptorType);
    	printk(KERN_EMERG "         Endpoint, bEndpointAddress=%d\r\n",endpoint_desc->bEndpointAddress);
    	printk(KERN_EMERG "         Endpoint, bmAttributes=%d\r\n",endpoint_desc->bmAttributes);
    	printk(KERN_EMERG "         Endpoint, wMaxPacketSize=%d\r\n",endpoint_desc->wMaxPacketSize);
   		printk(KERN_EMERG "         Endpoint, bInterval=%d\r\n",endpoint_desc->bInterval);
    	printk(KERN_EMERG "         Endpoint, bRefresh=%d\r\n",endpoint_desc->bRefresh);
    	printk(KERN_EMERG "         Endpoint, bSynchAddress=%d\r\n",endpoint_desc->bSynchAddress);
   }
#endif   

    /* set up the endpoint information */
    /* use only the first bulk-in and bulk-out endpoints */
    for (i = 0; i < host_intf->desc.bNumEndpoints; ++i) 
    {
        endpoint_desc = &host_intf->endpoint[i].desc;

    
        if (!dev->bulk_in_endpointAddr && usb_endpoint_is_bulk_in(endpoint_desc)) 
        {   /* we found a bulk in endpoint */
            buffer_size = le16_to_cpu(endpoint_desc->wMaxPacketSize);
            dev->bulk_in_size = buffer_size;
            dev->bulk_in_endpointAddr = endpoint_desc->bEndpointAddress;
            dev->bulk_in_buffer = kmalloc(buffer_size, GFP_KERNEL);
            if (!dev->bulk_in_buffer) 
            {
                err("Could not allocate bulk_in_buffer");
                goto error;
            }
            dev->bulk_in_urb = usb_alloc_urb(0, GFP_KERNEL);
            if (!dev->bulk_in_urb) 
            {
                err("Could not allocate bulk_in_urb");
                goto error;
            }
        }
    
        if (!dev->bulk_out_endpointAddr && usb_endpoint_is_bulk_out(endpoint_desc))
        {   /* we found a bulk out endpoint */
            dev->bulk_out_endpointAddr = endpoint_desc->bEndpointAddress;
        }
    }
    if (!(dev->bulk_in_endpointAddr && dev->bulk_out_endpointAddr)) 
    {
        err("Could not find both bulk-in and bulk-out endpoints");
        goto error;
    }
    
    usb_set_intfdata(interface, dev);
    
    /* we can register the device now, as it is ready */
    res = usb_register_dev(interface, &qsi_myspin_class);
    if (res) 
    {   /* something prevented us from registering this driver */
        printk(KERN_EMERG "kernel : Not able to get a minor for this device.\n");
        usb_set_intfdata(interface, NULL);
        goto error;
    }
     
    /* let the user know what node this device is now attached to */
    dev_info(&interface->dev, "USB qsi_mySPIN device now attached to qsi_myspin%d", interface->minor);
   
    if( id->idVendor == USB_VENDOR_ID_GOOGLE)
        return 0;

   // get protocol
   res = usb_control_msg ( dev->udev,
                           usb_rcvctrlpipe(dev->udev, dev->ctrl_epAddr),
                           AOA_REQ_PROTOCOL,
                           AOA_CTRL_IN|USB_RECIP_DEVICE,
                           0,
                           0,
                           proto,
                           2,
                           USB_CTRL_GET_TIMEOUT);

   printk("get AOA protocol version : %d.%d\n", proto[0], proto[1]);

   qsi_myspin_set_proto(dev, AOA_PROTO_MANUFACTURE_INDEX, "BSOT");
   qsi_myspin_set_proto(dev, AOA_PROTO_DESCRIPTION_INDEX, "Qsi mySpin");

   if(proto[0] >= 2)
   {         
	   if(qsi_myspin_set_audio_mode(dev, 1) < 0)
           proto[0] = 1;
   }
   else if(proto[0] < 1)
   {
       printk("AOA no support!!");
       return 0;
   }
 
   if(proto[0] == 1)
   {
	   qsi_myspin_set_proto(dev, AOA_PROTO_MODEL_INDEX,"accessory");
   	   qsi_myspin_set_proto(dev, AOA_PROTO_VERSION_INDEX, "1.0");
   }
 
   if(proto[0] == 2)
   {
       qsi_myspin_set_proto(dev, AOA_PROTO_MODEL_INDEX,"accessory + audio");
	   qsi_myspin_set_proto(dev, AOA_PROTO_VERSION_INDEX, "2.0");
   }
  
   qsi_myspin_set_proto(dev, AOA_PROTO_URI_INDEX, "com.bosch.myspin.launcherapp");
   qsi_myspin_set_proto(dev, AOA_PROTO_SERIAL_INDEX, "0000000012345678");
   
   res = qsi_myspin_switch_to_accessory_mode(dev);
   printk("start accessory mode %s\n",(res < 0) ? "error" : "ok");
  
    return 0;
error:
	if (dev)
    {	/* this frees allocated memory */
		kref_put(&dev->kref, qsi_myspin_delete);
    }
    return res;    
}

static void qsi_myspin_disconnect(struct usb_interface *interface)
{
	struct usb_qsi_myspin *dev;
	int minor = interface->minor;

	dev = usb_get_intfdata(interface);
	usb_set_intfdata(interface, NULL);

	/* give back our minor */
	usb_deregister_dev(interface, &qsi_myspin_class);

	/* prevent more I/O from starting */
	mutex_lock(&dev->io_mutex);
	dev->interface = NULL;
	mutex_unlock(&dev->io_mutex);

	usb_kill_anchored_urbs(&dev->submitted);

	/* decrement our usage count */
	kref_put(&dev->kref, qsi_myspin_delete);

	dev_info(&interface->dev, "mySPIN USB #%d now disconnected", minor);
}


static void qsi_myspin_draw_down(struct usb_qsi_myspin *dev)
{
	int time;

	time = usb_wait_anchor_empty_timeout(&dev->submitted, 1000);
	if (!time)
		usb_kill_anchored_urbs(&dev->submitted);
	usb_kill_urb(dev->bulk_in_urb);
}

static int qsi_myspin_suspend(struct usb_interface *intf, pm_message_t message)
{
	struct usb_qsi_myspin *dev = usb_get_intfdata(intf);

	if (!dev)
		return 0;
	qsi_myspin_draw_down(dev);
	return 0;
}

static int qsi_myspin_resume(struct usb_interface *intf)
{
	return 0;
}

static int qsi_myspin_pre_reset(struct usb_interface *intf)
{
	struct usb_qsi_myspin *dev = usb_get_intfdata(intf);

	mutex_lock(&dev->io_mutex);
	qsi_myspin_draw_down(dev);

	return 0;
}

static int qsi_myspin_post_reset(struct usb_interface *intf)
{
	struct usb_qsi_myspin *dev = usb_get_intfdata(intf);

	/* we are sure no URBs are active - no locking needed */
	dev->errors = -EPIPE;
	mutex_unlock(&dev->io_mutex);

	return 0;
}


static struct usb_driver qsi_myspin_driver = {
	.name       =   "qsi_myspin",
	.probe      =	qsi_myspin_probe,
	.disconnect =	qsi_myspin_disconnect,
	.suspend    =	qsi_myspin_suspend,
	.resume     =	qsi_myspin_resume,
	.pre_reset  =	qsi_myspin_pre_reset,
	.post_reset =	qsi_myspin_post_reset,
	.id_table   =	qsi_myspin_table,
	.supports_autosuspend = 1,
};

static int __init qsi_myspin_init(void)
{
	int result;

	/* register this driver with the USB subsystem */
	result = usb_register(&qsi_myspin_driver);
	if (result)
		err("usb_register failed. Error number %d", result);

	return result;
}

static void __exit qsi_myspin_exit(void)
{
	/* deregister this driver with the USB subsystem */
	usb_deregister(&qsi_myspin_driver);
}

module_init(qsi_myspin_init);
module_exit(qsi_myspin_exit);

MODULE_LICENSE("GPL");

