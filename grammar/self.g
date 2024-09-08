
set wsc = isspace;

let ws = 0_512wsc;

set alpha = isalpha; 

set idc = '_' + isalnum;

let idpat = alpha 0_256idc;

let id = pgc_lang_capid $ idpat;

set xdigit = isxdigit;

let xdigits = 1_2xdigit;

let xbyte = pgc_lang_capxbyte $ xdigits;

let pctbyte = '%' xbyte;

set print = isprint;

let charlit = %27 (pgc_lang_capchar $ print) %27;

set digit = isdigit; 

let digits = 1_10digit;

let num = pgc_lang_capnum $ digits;

let range = pgc_lang_caprange $ num '_' num; 

let utfvalue = pgc_lang_caputf $ 1_6xdigit; 

let utfrange = pgc_lang_caputfrange $ '&' utfvalue '_' utfvalue; 

dec setexp; 
     
let setparen = '(' setexp ')'; 

let setterm = setparen | pctbyte | charlit | id; 

let setunion = '+' setexp @pgc_lang_setunion; 

let setdiff = '-' setexp @pgc_lang_setdiff; 

let setop = setunion | setdiff; 

def setexp = pgc_lang_capsetexp $ ws setterm ws 0_1setop; 

dec exp; 

let paren = '(' exp ')';

let term = pctbyte | charlit | paren | id; 

let rep = pgc_lang_caprep $ range term;

let hook = '@' pgc_lang_caphook $ idpat; 

let phrase = rep | hook | utfrange | term; 

let or = '|' exp @pgc_lang_setor; 

let call = '$' exp @pgc_lang_setcall; 

let and = exp @pgc_lang_setand; 

let op = call | or | and; 
   
def exp = pgc_lang_capexp $ ws phrase ws 0_1op;

let wsidws = ws id ws;
     
let dec = 'd' 'e' 'c' wsidws ';' @pgc_lang_setdec; 

let wsidwseq = wsidws '='; 

let def = 'd' 'e' 'f' wsidwseq exp ';' @pgc_lang_setdef; 

let set = 's' 'e' 't' wsidwseq setexp ';' @pgc_lang_setset; 

let let = 'l' 'e' 't' wsidwseq exp ';' @pgc_lang_setlet; 

let stmt = pgc_lang_capstmt $ def | dec | let | set; 

let cmt = '#' 0_1024print; 
      
let src = ws 0_50000((cmt | stmt) ws); 
      