call_command ("forward-word", "2")
call_command ("insert", ")")
call_command ("beginning-of-line")
call_command ("insert", "(")
call_command ("beginning-of-line")
call_command ("kill-sexp")
call_command ("save-buffer")
call_command ("save-buffers-kill-emacs")