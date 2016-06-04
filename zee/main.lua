-- Program invocation, startup and shutdown
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

-- Derived constants
VERSION_STRING = PACKAGE_NAME .. " " .. VERSION

local COPYRIGHT_STRING = ""

local program_name = posix.basename (arg[0] or PACKAGE)
spec = program_name .. " " .. VERSION .. [[

Copyright (C) 2015 Free Software Foundation, Inc.
]] .. PACKAGE_NAME .. " comes with ABSOLUTELY NO WARRANTY." .. [[

You may redistribute copies of ]] .. PACKAGE_NAME .. [[


under the terms of the GNU General Public License.
For more information about these matters, see the file named COPYING.

Usage: ]] .. program_name .. [[


An editor.

Exit status is 0 if OK, 1 if it cannot start up, for example because
of an invalid command-line argument, and 2 if it crashes or runs out
of memory.

    ~/.]] .. PACKAGE .. [[ is the user init file

  -q, --no-init-file  do not load ~/.]] .. PACKAGE .. [[

  -e, --eval=CHUNK    evaluate Lua chunk CHUNK
  -n, --line=LINE     start editing at line LINE
      --version       display version information, then exit
      --help          display this help, then exit

Please report bugs at ]] .. PACKAGE_BUGREPORT .. [[
]]
--    -b, --batch          run non-interactively


-- Runtime constants

-- Display attributes
display = {}

-- Keyboard handling

GETKEY_DEFAULT = -1
GETKEY_DELAYED = 2000


-- Global flags, stored in thisflag and lastflag.
-- need_resync:    a resync is required.
-- quit:           the user has asked to quit.
-- defining_macro: we are defining a macro.


-- The current window
win = nil

-- The current buffer
buf = nil

-- The global editor flags.
thisflag = {}
lastflag = {}


local function segv_sig_handler (signo)
  io.stderr:write (prog.name .. ": " .. PACKAGE_NAME ..
                   " crashed. Please send a bug report to <" ..
                   PACKAGE_BUGREPORT .. ">.\r\n")
  editor_exit (true)
end

local function other_sig_handler (signo)
  local msg = progran_name .. ": terminated with signal " .. signo .. ".\n" .. debug.traceback ()
  io.stderr:write (msg:gsub ("\n", "\r\n"))
  editor_exit (false)
end

local function signal_init ()
  -- Set up signal handling
  posix.signal(posix.SIGSEGV, segv_sig_handler)
  posix.signal(posix.SIGBUS, segv_sig_handler)
  posix.signal(posix.SIGHUP, other_sig_handler)
  posix.signal(posix.SIGINT, other_sig_handler)
  posix.signal(posix.SIGTERM, other_sig_handler)
end

function main ()
  signal_init ()
  local OptionParser = require "std.optparse"
  local parser = OptionParser (spec)
  _G.arg, _G.opts = parser:parse (_G.arg)

  if #arg ~= 1 and not (#arg == 0 and opts.eval) then
    parser:opterr ("Need a file or expression")
  end

  os.setlocale ("")
  win = {}

  local w, h = term_width (), term_height ()
  win = {topdelta = 0, start_column = 0, last_line = 0}
  win.fwidth = 2
  term_init ()
  resize_window ()

  if not opts.no_init_file then
    local s = os.getenv ("HOME")
    if s then
      execute_command ("load", s .. "/." .. PACKAGE)
    end
  end

  -- Load file
  local ok = true
  if #arg == 1 then
    ok = not read_file (arg[1])
    if ok then
      execute_command ("move-goto-line", opts.line or 1)
    end
  end

  -- Evaluate Lua chunks given on the command line.
  if type (opts.eval) == "string" then -- If only one argument, put it in a table
    opts.eval = {opts.eval}
  end
  for _, c in ipairs (opts.eval or {}) do
    if execute_command ("eval", c) then
      break
    end
    if thisflag.quit then
      break
    end
  end

  if ok and #arg == 1 then
    lastflag.need_resync = true

    -- Refresh minibuffer in case there's a pending error message.
    minibuf_refresh ()

    -- Leave cursor in correct position.
    term_redraw_cursor ()

    -- Run the main loop.
    while not thisflag.quit do
      if lastflag.need_resync then
        window_resync (win)
      end
      get_and_run_command ()
    end
  end

  -- Tidy and close the terminal.
  term_finish ()

  -- Print any error message.
  if not ok then
    io.stderr:write (minibuf_contents .. "\n")
  end
  -- FIXME: Add startup banner (how to quit and get menu)
end
