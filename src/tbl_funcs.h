/* Interactive commands

   Copyright (c) 2008 Free Software Foundation, Inc.
   Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004 Sandro Sigala.
   Copyright (c) 2003, 2004, 2005, 2006, 2007, 2008 Reuben Thomas.
   Copyright (c) 2004 David Capello.

   This file is part of GNU Zile.

   GNU Zile is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   GNU Zile is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GNU Zile; see the file COPYING.  If not, write to the
   Free Software Foundation, Fifth Floor, 51 Franklin Street, Boston,
   MA 02111-1301, USA.  */

/*
 * Add an entry to this list for declaring a new interactive function,
 * in alphabetic order. X0 means no key binding, X1 means one key
 * binding, etc.; XX is for a non-interactive function.
 *
 * Please remember to keep in sync with the Texinfo documentation
 * `../doc/zile.texi', and to keep functions lexically sorted!
 */

X0 ("auto-fill-mode", auto_fill_mode)
X1 ("back-to-indentation", back_to_indentation, "\\M-m")
X2 ("backward-char", backward_char, "\\C-b", "\\LEFT")
X1 ("backward-delete-char", backward_delete_char, "\\BACKSPACE")
X1 ("backward-kill-word", backward_kill_word, "\\M-\\BACKSPACE")
X1 ("backward-paragraph", backward_paragraph, "\\M-{")
X1 ("backward-sexp", backward_sexp, "\\C-\\M-b")
X1 ("backward-word", backward_word, "\\M-b")
X1 ("beginning-of-buffer", beginning_of_buffer, "\\M-<")
X2 ("beginning-of-line", beginning_of_line, "\\C-a", "\\HOME")
X1 ("call-last-kbd-macro", call_last_kbd_macro, "\\C-xe")
X1 ("capitalize-word", capitalize_word, "\\M-c")
X0 ("cd", cd)
X1 ("copy-region-as-kill", copy_region_as_kill, "\\M-w")
X2 ("copy-to-register", copy_to_register, "\\C-xrx", "\\C-xrs")
X1 ("delete-blank-lines", delete_blank_lines, "\\C-x\\C-o")
X2 ("delete-char", delete_char, "\\C-d", "\\DELETE")
X1 ("delete-horizontal-space", delete_horizontal_space, "\\M-\\\\")
X1 ("delete-other-windows", delete_other_windows, "\\C-x1")
X0 ("delete-region", delete_region)
X1 ("delete-window", delete_window, "\\C-x0")
X2 ("describe-bindings", describe_bindings, "\\C-hb", "\\F1b")
X2 ("describe-function", describe_function, "\\C-hf", "\\F1f")
X2 ("describe-key", describe_key, "\\C-hk", "\\F1k")
X2 ("describe-variable", describe_variable, "\\C-hv", "\\F1v")
X1 ("downcase-region", downcase_region, "\\C-x\\C-l")
X1 ("downcase-word", downcase_word, "\\M-l")
X1 ("end-kbd-macro", end_kbd_macro, "\\C-x)")
X1 ("end-of-buffer", end_of_buffer, "\\M->")
X2 ("end-of-line", end_of_line, "\\C-e", "\\END")
X1 ("enlarge-window", enlarge_window, "\\C-x^")
X1 ("exchange-point-and-mark", exchange_point_and_mark, "\\C-x\\C-x")
X1 ("execute-extended-command", execute_extended_command, "\\M-x")
X1 ("fill-paragraph", fill_paragraph, "\\M-q")
X1 ("find-alternate-file", find_alternate_file, "\\C-x\\C-v")
X1 ("find-file", find_file, "\\C-x\\C-f")
X1 ("find-file-read-only", find_file_read_only, "\\C-x\\C-r")
X2 ("forward-char", forward_char, "\\C-f", "\\RIGHT")
X0 ("forward-line", forward_line)
X1 ("forward-paragraph", forward_paragraph, "\\M-}")
X1 ("forward-sexp", forward_sexp, "\\C-\\M-f")
X1 ("forward-word", forward_word, "\\M-f")
X0 ("global-set-key", global_set_key)
X0 ("goto-char", goto_char)
X2 ("goto-line", goto_line, "\\M-gg", "\\M-g\\M-g")
X3 ("help", help, "\\C-hh", "\\F1h", "\\F1\\F1")
X2 ("help-with-tutorial", help_with_tutorial, "\\C-ht", "\\F1t")
X1 ("indent-for-tab-command", indent_for_tab_command, "\\TAB")
X0 ("indent-relative", indent_relative)
X0 ("insert-buffer", insert_buffer)
X1 ("insert-file", insert_file, "\\C-xi")
X2 ("insert-register", insert_register, "\\C-xrg", "\\C-xri")
X1 ("isearch-backward", isearch_backward, "\\C-r")
X1 ("isearch-backward-regexp", isearch_backward_regexp, "\\C-\\M-r")
X1 ("isearch-forward", isearch_forward, "\\C-s")
X1 ("isearch-forward-regexp", isearch_forward_regexp, "\\C-\\M-s")
X1 ("just-one-space", just_one_space, "\\M-\\SPC")
X1 ("keyboard-quit", keyboard_quit, "\\C-g")
X1 ("kill-buffer", kill_buffer, "\\C-xk")
X1 ("kill-line", kill_line, "\\C-k")
X1 ("kill-region", kill_region, "\\C-w")
X1 ("kill-sexp", kill_sexp, "\\C-\\M-k")
X1 ("kill-word", kill_word, "\\M-d")
X1 ("list-buffers", list_buffers, "\\C-x\\C-b")
X0 ("list-registers", list_registers)
X1 ("mark-paragraph", mark_paragraph, "\\M-h")
X1 ("mark-sexp", mark_sexp, "\\C-\\M-@")
X1 ("mark-whole-buffer", mark_whole_buffer, "\\C-xh")
X1 ("mark-word", mark_word, "\\M-@")
X0 ("name-last-kbd-macro", name_last_kbd_macro)
X1 ("newline", newline, "\\RET")
X1 ("newline-and-indent", newline_and_indent, "\\C-j")
X2 ("next-line", next_line, "\\C-n", "\\DOWN")
X1 ("open-line", open_line, "\\C-o")
X1 ("other-window", other_window, "\\C-xo")
X1 ("overwrite-mode", overwrite_mode, "\\INSERT")
X2 ("previous-line", previous_line, "\\C-p", "\\UP")
X1 ("query-replace", query_replace, "\\M-%")
X1 ("quoted-insert", quoted_insert, "\\C-q")
X1 ("recenter", recenter, "\\C-l")
X1 ("save-buffer", save_buffer, "\\C-x\\C-s")
X1 ("save-buffers-kill-zile", save_buffers_kill_zile, "\\C-x\\C-c")
X1 ("save-some-buffers", save_some_buffers, "\\C-xs")
X2 ("scroll-down", scroll_down, "\\M-v", "\\PRIOR")
X2 ("scroll-up", scroll_up, "\\C-v", "\\NEXT")
X0 ("search-backward", search_backward)
X0 ("search-backward-regexp", search_backward_regexp)
X0 ("search-forward", search_forward)
X0 ("search-forward-regexp", search_forward_regexp)
X0 ("self-insert-command", self_insert_command)
X1 ("set-fill-column", set_fill_column, "\\C-xf")
X1 ("set-mark-command", set_mark_command, "\\C-@")
X0 ("set-variable", set_variable)
XX ("setq", setq)
X1 ("shell-command", shell_command, "\\M-!")
X1 ("shell-command-on-region", shell_command_on_region, "\\M-|")
X0 ("shrink-window", shrink_window)
X1 ("split-window", split_window, "\\C-x2")
X1 ("start-kbd-macro", start_kbd_macro, "\\C-x(")
X2 ("suspend-zile", suspend_zile, "\\C-x\\C-z", "\\C-z")
X1 ("switch-to-buffer", switch_to_buffer, "\\C-xb")
X1 ("tab-to-tab-stop", tab_to_tab_stop, "\\M-i")
X0 ("tabify", tabify)
X1 ("toggle-read-only", toggle_read_only, "\\C-x\\C-q")
X0 ("transient-mark-mode", transient_mark_mode)
X1 ("transpose-chars", transpose_chars, "\\C-t")
X1 ("transpose-lines", transpose_lines, "\\C-x\\C-t")
X1 ("transpose-sexps", transpose_sexps, "\\C-\\M-t")
X1 ("transpose-words", transpose_words, "\\M-t")
X2 ("undo", undo, "\\C-xu", "\\C-_")
X1 ("universal-argument", universal_argument, "\\C-u")
X0 ("untabify", untabify)
X1 ("upcase-region", upcase_region, "\\C-x\\C-u")
X1 ("upcase-word", upcase_word, "\\M-u")
X2 ("view-zile-FAQ", view_zile_FAQ, "\\C-h\\C-f", "\\F1\\C-f")
X2 ("where-is", where_is, "\\C-hw", "\\F1w")
X1 ("write-file", write_file, "\\C-x\\C-w")
X1 ("yank", yank, "\\C-y")
X0 ("zile-version", zile_version)
