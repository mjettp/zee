(mark-word)
(kill-region (point) (mark))
(end-of-line)
(yank)
(save-buffer)
(save-buffers-kill-emacs)
