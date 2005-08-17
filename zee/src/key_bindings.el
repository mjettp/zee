# Default key bindings

# Navigation. Mostly arrows and things.
# Ideally everything should have a home-keys alternative, but this is
# currently not true.

# Character/line
global-set-key "LEFT" 'edit-navigate-backward-char
global-set-key "C-b" 'edit-navigate-backward-char
global-set-key "RIGHT" 'edit-navigate-forward-char
# FIXME: Need a home-keys shortcut for edit-navigate-forward-char. Was C-f.
global-set-key "DOWN" 'edit-navigate-down-line
global-set-key "C-n" 'edit-navigate-down-line
global-set-key "UP" 'edit-navigate-up-line
global-set-key "C-p" 'edit-navigate-up-line
# Word/paragraph
global-set-key "C-LEFT" 'backward-word
global-set-key "M-b" 'backward-word
global-set-key "C-RIGHT" 'forward-word
global-set-key "M-f" 'forward-word
global-set-key "M-{" 'backward-paragraph
global-set-key "M-}" 'forward-paragraph
# Line/page
global-set-key "C-a" 'beginning-of-line
global-set-key "HOME" 'beginning-of-line
global-set-key "C-e" 'end-of-line
global-set-key "END" 'end-of-line
global-set-key "PGUP" 'scroll-down
global-set-key "PGDN" 'scroll-up
# Whole buffer
global-set-key "M-<" 'beginning-of-buffer
global-set-key "C-HOME" 'beginning-of-buffer
global-set-key "M->" 'end-of-buffer
global-set-key "C-END" 'end-of-buffer

# Selection. The goal is that any navigation command can be combined with
# SHIFT to form a selection command.

# FIXME: Not done yet.

# Long distance navigation. These are like navigation commands but they don't
# become selection commands when combined with SHIFT.

global-set-key "C-M-g" 'goto-char
global-set-key "M-g" 'goto-line

# Open, save, close, quit etc.

# Open
global-set-key "C-o" 'file-open
global-set-key "C-p" 'file-switch
global-set-key "C-M-x" 'file-switch
# Save
global-set-key "C-s" 'file-save
# Close
global-set-key "C-w" 'file-close
# Quit
global-set-key "M-C-q" 'file-quit
global-set-key "M-q" 'file-quit
global-set-key "C-q" 'file-quit

# Undo, cut, copy, paste, delete etc.

# Undo
global-set-key "C--" 'undo
global-set-key "C-z" 'undo
# Cut selection to clipboard.
global-set-key "C-x" 'kill-region
global-set-key "C-k" 'kill-region
global-set-key "M-d" 'kill-word
global-set-key "M-DEL" 'kill-word
# Copy selection to clipboard.
global-set-key "C-c" 'copy-region-as-kill
global-set-key "M-w" 'copy-region-as-kill
# Delete without modifying clipboard.
global-set-key "BS" 'backward-delete-char
global-set-key "M-BS" 'backward-kill-word
global-set-key "C-d" 'delete-char
global-set-key "DEL" 'delete-char
# Paste.
global-set-key "C-y" 'yank
global-set-key "C-v" 'yank

# Search

# Once the search is underway, "find next" is hard-wired to C-i.
# Having it hard-wired is obviously broken, but something neutral like RET
# would be better.
# The proposed meaning of ESC obviates the current behaviour of RET.
global-set-key "C-f" 'isearch-forward-regexp

# Window management.

global-set-key "M-n" 'window-next
global-set-key "M-o" 'window-split
global-set-key "M-p" 'window-close-others
global-set-key "M-w" 'window-close

global-set-key "ESC" 'keyboard-quit
global-set-key "C-g" 'keyboard-quit

# Insert special characters.

global-set-key "C-RET" 'newline
global-set-key "RET" 'newline-and-indent
global-set-key "TAB" 'indent-relative
# quoted-insert needs a new shortcut? Was C-q.

# Rare commands.
# SUGGESTION: remove these from the default keymap?

global-set-key "M-m" 'back-to-indentation
global-set-key "C-M-b" 'backward-sexp
global-set-key "M-e" 'call-last-kbd-macro
global-set-key "M-c" 'capitalize-word
global-set-key "M-\" 'delete-horizontal-space
global-set-key "M-1" 'window-close-others
global-set-key "M- " 'just-one-space
global-set-key "M-l" 'downcase-word
global-set-key "M-)" 'end-kbd-macro
global-set-key "M-:" 'eval-expression
global-set-key "C-M-e" 'eval-last-sexp
global-set-key "M-x" 'execute-extended-command
global-set-key "M-q" 'fill-paragraph
global-set-key "C-M-f" 'forward-sexp
global-set-key "C-r" 'isearch-backward-regexp
global-set-key "C-M-k" 'kill-sexp
global-set-key "M-h" 'mark-paragraph
global-set-key "C-M-@" 'mark-sexp
global-set-key "M-@" 'mark-word
global-set-key "C-j" 'newline
global-set-key "M-%" 'query-replace-regexp
global-set-key "C-l" 'recenter
global-set-key "C-@" 'set-mark-command
global-set-key "M-!" 'shell-command
global-set-key "M-(" 'start-kbd-macro
global-set-key "M-i" 'tab-to-tab-stop
global-set-key "C-u" 'universal-argument
global-set-key "M-u" 'upcase-word
