call_command ("describe-function", "forward-char")
call_command ("other-window", "1")
call_command ("set-mark", "point")
call_command ("forward-line", "1")
call_command ("forward-word", "-4")
call_command ("copy-region-as-kill", "mark", "point")
call_command ("other-window", "-1")
call_command ("yank")
call_command ("save-buffer")
call_command ("save-buffers-kill-emacs")