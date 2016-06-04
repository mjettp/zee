-- User commands
--
-- Copyright (c) 2009-2016 Free Software Foundation, Inc.
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
-- along with this program; see the file COPYING.  If not, write to the
-- Free Software Foundation, Fifth Floor, 51 Franklin Street, Boston,
-- MA 02111-1301, USA.


-- User things
env = {}

-- Turn texinfo markup into plain text
local function texi (s)
  s = s:gsub ("@i{([^}]+)}", function (s) return s:upper () end)
  s = s:gsub ("@kbd{([^}]+)}", "%1") -- FIXME: look up binding of command
  return s
end

function Define (name, doc, value)
  env[name] = {
    doc = texi (doc:chomp ()),
    val = value,
  }
end

function get_doc (name)
  if command_exists (name) then
    return env[name].doc
  elseif env[name] and env[name].doc then
    return (string.format ("%s is a variable.\n\nIts value is %s\n\n%s",
                           name, get_variable (name), env[name].doc))
  end
end

function execute_command (name, ...)
  return command_exists (name) and env[name].val (...)
end

local function loadchunk (func, err)
  if func == nil then
    minibuf_error (string.format ("Error evaluating Lua: %s", err))
    return true
  end
  return func ()
end

Define ("eval",
[[
Evaluate a Lua chunk CHUNK.
]],
  function (chunk)
    return loadchunk (load (chunk))
  end
)

Define ("load",
[[
Load and evaluate a Lua chunk from FILE.
]],
  function (file)
    return loadchunk (loadfile (file))
  end
)

function command_exists (c)
  return env[c] and type (env[c].val) == "function"
end


-- FIXME: Make this non-interactive: need to change Define so it
-- defines functions in _G, and hence can accept non-interactive
-- functions (which still need documentation, but should not appear in
-- menu).
Define ("execute-command",
[[
Read command name, then run it.
]],
  function ()
    local name = minibuf_read_completion ("Command: ",
                                          completion_new (filter (function (e)
                                                                    return command_exists (e)
                                                                  end,
                                                                  std.ielems, table.keys (env))),
                                          "command")
    if name == "" then
      return true
    end
    return execute_command (name)
  end
)
