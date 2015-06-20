-- Key bindings
--
-- Copyright (c) 2010-2015 Free Software Foundation, Inc.
--
-- This file is part of Zee.
--
-- This program is free software; you can redistribute it and/or modify it
-- under the terms of the GNU General Public License as published by
-- the Free Software Foundation; either version 3, or (at your option)
-- any later version.
--
-- This program is distributed in the hope that it will be useful, but
-- WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
-- General Public License for more details.
--
-- You should have received a copy of the GNU General Public License
-- along with this program.  If not, see <http://www.gnu.org/licenses/>.

-- Key binding.

_last_command = nil
_this_command = nil
_interactive = 0

function interactive ()
  return _interactive > 0
end

function call_command (f, ...)
  thisflag = {defining_macro = lastflag.defining_macro}

  -- Execute the command.
  _this_command = f
  _interactive = _interactive + 1
  local ok = not execute_command (f, ...) -- FIXME: Most of this (except _interactive) should be inside execute_command
  _interactive = math.max (0, _interactive - 1)
  _last_command = _this_command

  if buf and _last_command ~= "edit-undo" then
    buf.next_undo = buf.last_undo
  end

  lastflag = thisflag

  return ok
end

function get_and_run_command ()
  local key = get_key_chord ()
  local name = binding_to_command (key)

  popup_clear ()
  minibuf_clear ()

  if command_exists (name) then
    call_command (name)
  else
    minibuf_error (tostring (key) .. " is undefined")
  end
end

root_bindings = {}
bindings = root_bindings

local function key_canon (keystr)
  local key = tostring (keycode (keystr)) -- canonicalize the string
  if not key then
    minibuf_error (string.format ("Key sequence %s is invalid", keystr))
    return
  end
  return key
end

function key_unbind (keystr)
  local key = key_canon (keystr)
  if key then
    bindings[key] = nil
  end
end

function key_bind (keystr, cmd)
  local key = key_canon (keystr)
  if key then
    if not command_exists (cmd) then -- Possible if called non-interactively
      minibuf_error (string.format ("No such function `%s'", cmd))
      return
    end

    bindings[key] = cmd
  end
end

function bind_printing_chars (cmd)
  -- Bind all printing keys to given command
  for i = 0, 0xff do
    if posix.isprint (string.char (i)) then
      bindings[string.char (i)] = cmd
    end
  end
  -- Bind special key names to edit-insert-character
  functional.map (function (e)
                    bindings[tostring (keycode (e))] = cmd
                  end,
                  std.ielems,
                  {"Space", "Tab", "Return"})
end

bind_printing_chars ("edit-insert-character")
-- FIXME: Make Escape bindable and make it possible to escape to top level

function binding_to_command (key)
  return bindings[tostring (key)]
end

function command_to_binding (cmd)
  local keys = {}
  for k, n in pairs (bindings) do
    if n == cmd then
      table.insert (keys, k)
    end
  end
  return keys
end
