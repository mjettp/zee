(forward-word)
(insert "(")
(forward-word)
(forward-word)
(forward-word)
(insert ")")
(backward-word)
(backward-word)
(backward-char)
(backward-char)
(backward-char)
(backward-char)
(backward-char)
(mark-sexp)
(kill-region (point) (mark))
(save-buffer)
(save-buffers-kill-emacs)
