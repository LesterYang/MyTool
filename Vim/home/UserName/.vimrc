""""""""""""""
" vim setting
""""""""""""""
if has("autocmd")
  filetype plugin indent on
endif

set nu			    " display line number
set autoindent		" auto indentation
set incsearch		" incremental search
set hlsearch		" search highlightin
set nobackup		" no *~ backup files
set copyindent		" copy the previous indentation on autoindenting
set ignorecase		" ignore case when searching
set smartcase		" ignore case if search pattern is all lowercase,case-sensitive otherwise
set smarttab		" insert tabs on the start of a line according to context
set showtabline=0
set mouse=a	        " Enable mouse usage (all modes)
set nocompatible	" not compatible with the old-fashion vi mode
set bs=2	        " allow backspacing over everything in insert mode
set history=50		" keep 50 lines of command line history
set ruler	        " show the cursor position all the time
set autoread		" auto read when file is changed from outside
set expandtab           " replace <TAB> with spaces
set softtabstop=4 
set shiftwidth=4 
set tags+=~/.vim/systags "add system tags
set fileencodings=utf-8,gbk,ucs-bom,cp936

"""""""""""""""
" color scheme
"""""""""""""""
set t_Co=256
syntax on
"set background=dark
"colorscheme qsicol
colorscheme molokai

""""""""""""""
" status line
""""""""""""""
hi User1 ctermfg=15    ctermbg=21 cterm=bold
hi User2 ctermfg=21    ctermbg=27
hi User3 ctermfg=15    ctermbg=27 
hi User4 ctermfg=27    ctermbg=33 
hi User5 ctermfg=15    ctermbg=33 cterm=bold
hi User6 ctermfg=33    ctermbg=45 
hi User7 ctermfg=0     ctermbg=45
hi User8 ctermfg=0     ctermbg=49

set laststatus=2
set statusline=%<%1*
set statusline+=\ %t\ 
set statusline+=%2*▶\ %3*
set statusline+=%h%m%r\ 
set statusline+=%4*▶\ %5*
set statusline+=%{fugitive#statusline()}\ 
set statusline+=%6*▶\ %7*
set statusline+=%=%-14.(%l,%c%V%)[%{&encoding}]
set statusline+=%8*
set statusline+=\ %P
hi StatusLine term=bold,reverse cterm=bold,reverse


""""""""""""""""""""""""""""""""""""""""""""""""""
" Source a global configuration file if available
""""""""""""""""""""""""""""""""""""""""""""""""""
if filereadable("/etc/vim/vimrc.local")
  source /etc/vim/vimrc.local
endif

let g:Tb_MaxSize=2
let g:Tb_TabWrap=1

""""""""
" remap
""""""""
inoremap {<CR> {<CR><END><CR>}<UP><END>
imap .. .<C-X><C-O>
imap -- -><C-X><C-O>
autocmd CursorMovedI * if pumvisible() == 0|pclose|endif
autocmd InsertLeave * if pumvisible() == 0|pclose|endif

vmap <Tab> >gv
vmap <S-Tab> <gv

let mapleader=","
let g:mapleader=","

map <leader>cc :botright cope<CR>                       " open the error console
map <leader>]  :cn<CR>                                  " move to next error
map <leader>[  :cp<CR>                                  " move to the prev error
nmap <leader>/ :nohl<CR>                                " turn off search highlighting
"nmap <leader>h :UpdateTypesFile<CR>                     " update taghighlight
nmap <leader>s :w<CR>                                   " save  

fun! Replace() 
    let s:word = input("Replace " . expand('<cword>') . " with:") 
    :exe 'bufdo! %s/\<' . expand('<cword>') . '\>/' . s:word . '/ge' 
    :unlet! s:word 
endfun 

" replace the current word in all opened buffers
map <leader>r :call Replace()<CR>

" ,g search recursively
map <leader>g :vimgrep //j **<left><left><left><left><left>

inoremap <C-A> <Home>
inoremap <C-E> <End>
inoremap <C-h> <LEFT>
inoremap <C-j> <DOWN>
inoremap <C-k> <UP>
inoremap <C-l> <RIGHT>
inoremap <C-f> <PageDown>
inoremap <C-b> <PageUP>
cnoremap <C-A> <Home>
cnoremap <C-E> <End>
cnoremap <C-K> <C-U>

nnoremap <C-j> <C-w>j
nnoremap <C-k> <C-w>k
nnoremap <C-h> <C-w>h
nnoremap <C-l> <C-w>l


"""""""""""""""""
" taglist plugin
"""""""""""""""""
"nnoremap <silent> <F4> :TlistToggle<CR> 
"let Tlist_Show_One_File = 1
"let Tlist_Auto_Open = 1
"let Tlist_Auto_Update = 1
"let Tlist_Exit_OnlyWindow = 1
"let Tlist_WinWidth = 24

""""""""""""""""""
" NerdTree plugin
""""""""""""""""""
nnoremap <silent> <F4> :NERDTree<CR> 
let NERDTreeWinPos="left"
let NERDTreeWinSize=24
let NERDTreeShowBookmarks=1

"au VimEnter *  NERDTree

""""""""""""""""""
" OmniCppComplete
""""""""""""""""""
let OmniCpp_NamespaceSearch = 1
let OmniCpp_GlobalScopeSearch = 1
let OmniCpp_ShowAccess = 1
let OmniCpp_ShowPrototypeInAbbr = 1                     " show function parameters
let OmniCpp_MayCompleteDot = 1                          " autocomplete after .
let OmniCpp_MayCompleteArrow = 1                        " autocomplete after ->
let OmniCpp_MayCompleteScope = 1                        " autocomplete after ::
let OmniCpp_DefaultNamespaces = ["std", "_GLIBCXX_STD"]

""""""""""""""""""
" EasyMotion
""""""""""""""""""
let g:EasyMotion_keys = 'asdfghjkl'
let g:EasyMotion_do_shade = 1
let g:EasyMotion_leader_key = '<Leader>'

""""""""""""""""""
" Tagbar
""""""""""""""""""
nmap <F8> :TagbarToggle<CR>
let g:tagbar_left=1
let g:tagbar_autoclose=1
let g:tagbar_expand=1
let g:tagbar_width=24

""""""""""""""""""
" Cscope
""""""""""""""""""
"map <F5> :!cscope -Rbq &<CR>:cs reset<CR><CR>
map <F5> :!~/.vim/cscope_update &<CR>:cs reset<CR><CR>
