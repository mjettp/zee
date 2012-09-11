-- Disk file handling
--
-- Copyright (c) 2009-2012 Free Software Foundation, Inc.
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

-- FIXME: Warn when file changes on disk

function exist_file (filename)
  if posix.stat (filename) then
    return true
  end
  local _, err = posix.errno ()
  return err ~= posix.ENOENT
end

local function is_regular_file (filename)
  local st = posix.stat (filename)

  if st and st.type == "regular" then
    return true
  end
end

-- Return nonzero if file exists and can be written.
local function check_writable (filename)
  local ok = posix.euidaccess (filename, "w")
  return ok and ok >= 0
end

-- This functions makes the passed path an absolute path:
--
--  * expands `~/' and `~name/' expressions;
--  * replaces `//' with `/' (restarting from the root directory);
--  * removes `..' and `.' entries.
--
-- Returns normalized path, or nil if a password entry could not be
-- read
function normalize_path (path)
  local comp = io.splitdir (path)
  local ncomp = {}

  -- Prepend cwd if path is relative
  if comp[1] ~= "" then
    comp = list.concat (io.splitdir (posix.getcwd () or ""), comp)
  end

  -- Deal with `~[user]', `..', `.', `//'
  for i, v in ipairs (comp) do
    if v == "" and i > 1 and i < #comp then -- `//'
      ncomp = {}
    elseif v == ".." then -- `..'
      table.remove (ncomp)
    elseif v ~= "." then -- not `.'
      if v[1] == "~" then -- `~[user]'
        ncomp = {}
        v = posix.getpasswd (v:match ("^~(.+)$"), "dir")
        if v == nil then
          return nil
        end
      end
      table.insert (ncomp, v)
    end
  end

  return io.catdir (unpack (ncomp))
end

-- Return a `~/foo' like path if the user is under his home directory,
-- else the unmodified path.
-- If the user's home directory cannot be read, nil is returned.
function compact_path (path)
  local home = posix.getpasswd (nil, "dir")
  -- If we cannot get the home directory, return empty string
  if home == nil then
    return ""
  end

  -- Replace `^$HOME' (if found) with `~'.
  return (string.gsub (path, "^" .. home, "~"))
end

Defun ("insert-file",
       {"string"},
[[
Insert contents of file FILENAME into buffer after point.
Set mark after the inserted text.
]],
  true,
  function (file)
    local ok = true

    if warn_if_readonly_buffer () then
      return false
    end

    if not file then
      file = minibuf_read_filename ("Insert file: ", cur_bp.dir)
      if not file then
        ok = execute_function ("keyboard-quit")
      end
    end

    if not file or file == "" then
      ok = false
    end

    if ok then
      local s = io.slurp (file)
      if s then
        insert_astr (AStr (s))
      else
        ok = minibuf_error ("%s: %s", file, posix.errno ())
      end
    else
      execute_function ("set-mark-command")
    end

    return ok
  end
)

-- Write buffer to given file name with given mode.
local function write_to_disk (bp, filename, mode)
  local ret = true
  local h = posix.creat (filename, mode)
  if not h then
    return false
  end

  local s = get_buffer_pre_point (bp)
  local written = posix.write (h, s)
  if written < 0 or written ~= #s then
    ret = written
  else
    s = get_buffer_post_point (bp)
    written = posix.write (h, s)
    if written < 0 or written ~= #s then
      ret = written
    end
  end

  if posix.close (h) ~= 0 then
    ret = false
  end

  return ret
end

-- Write the buffer contents to a file.
local function write_file (bp, filename)
  local ret, err = write_to_disk (bp, filename, "rw-rw-rw-")
  if ret then
    return true
  end

  if ret == -1 then
    return minibuf_error (string.format ("Error writing `%s': %s", filename, err))
  end
  return minibuf_error (string.format ("Error writing `%s'", filename))
end

local function write_buffer (bp, needname, confirm, name, prompt)
  local ans = true
  local ok = true

  if needname then
    name = minibuf_read_filename (prompt, "")
    if not name then
      return execute_function ("keyboard-quit")
    end
    if name == "" then
      return false
    end
    confirm = true
  end

  if confirm and exist_file (name) then
    ans = minibuf_read_yn (string.format ("File `%s' exists; overwrite? (y or n) ", name))
    if ans == -1 then
      execute_function ("keyboard-quit")
    elseif ans == false then
      minibuf_error ("Canceled")
    end
    if ans ~= true then
      ok = false
    end
  end

  if ans == true then
    if not bp.filename or name ~= bp.filename then
      set_buffer_names (bp, name)
    end
    bp.needname = false
    bp.temporary = false
    bp.nosave = false
    if write_file (bp, name) then
      minibuf_write ("Wrote " .. name)
      bp.modified = false
      undo_set_unchanged (bp.last_undop)
    else
      ok = false
    end
  end

  return ok
end

local function save_buffer (bp)
  if bp.modified then
    return write_buffer (bp, bp.needname, false, bp.filename, "File to save in: ")
  end

  minibuf_write ("(No changes need to be saved)")
  return true
end

Defun ("file-save",
       {},
[[
Save current buffer in visited file if modified.
]],
  true,
  function ()
    return save_buffer (cur_bp)
  end
)

Defun ("write-file",
       {},
[[
Write current buffer into file @i{filename}.
This makes the buffer visit that file, and marks it as not modified.

Interactively, confirmation is required unless you supply a prefix argument.
]],
  true,
  function ()
    return write_buffer (cur_bp, true,
                         _interactive and not lastflag.set_uniarg,
                         nil, "Write file: ")
  end
)

local function save_some_buffers ()
  local none_to_save = true
  local noask = false

  for _, bp in ripairs (buffers) do
    if bp.modified and not bp.nosave then
      local fname = get_buffer_filename_or_name (bp)

      none_to_save = false

      if noask then
        save_buffer (bp)
      else
        while true do
          minibuf_write (string.format ("Save file %s? (y, n, !, ., q) ", fname))
          local c = getkey (GETKEY_DEFAULT)
          minibuf_clear ()

          if c == keycode "\\C-g" then
            execute_function ("keyboard-quit")
            return false
          elseif c == keycode "q" then
            bp = nil
            break
          elseif c == keycode "." then
            save_buffer (bp)
            return true
          elseif c == keycode "!" then
            noask = true
          end
          if c == keycode "!" or c == keycode " " or c == keycode "y" then
            save_buffer (bp)
          end
          if c == keycode "!" or c == keycode " " or c == keycode "y" or c == keycode "n" or c == keycode "\\RET" or c == keycode "\\DELETE" then
            break
          else
            minibuf_error ("Please answer y, n, !, . or q.")
            waitkey (WAITKEY_DEFAULT)
          end
        end
      end
    end
  end

  if none_to_save then
    minibuf_write ("(No files need saving)")
  end

  return true
end

Defun ("file-quit",
       {},
[[
Offer to save each buffer, then kill this process.
]],
  true,
  function ()
    if not save_some_buffers () then
      return false
    end

    for _, bp in ipairs (buffers) do
      if bp.modified and not bp.needname then
        while true do
          local ans = minibuf_read_yesno ("Modified buffers exist; exit anyway? (yes or no) ")
          if ans == nil then
            return execute_function ("keyboard-quit")
          elseif not ans then
            return false
          end
          break -- We have found a modified buffer, so stop.
        end
      end
    end

    thisflag.quit = true
  end
)

function find_file (filename)
  local bp

  if exist_file (filename) and not is_regular_file (filename) then
    return minibuf_error ("File exists but could not be read")
  else
    bp = buffer_new ()
    set_buffer_names (bp, filename)
    bp.dir = posix.dirname (filename)

    local s = io.slurp (filename)
    if s then
      bp.readonly = not check_writable (filename)
    else
      s = ""
    end
    bp.text = AStr (s)

    -- Reset undo history
    bp.next_undop = nil
    bp.last_undop = nil
    bp.modified = false
  end

  switch_to_buffer (bp)
  thisflag.need_resync = true

  return true
end

-- Function called on unexpected error or crash (SIGSEGV).
-- Attempts to save modified buffers.
-- If doabort is true, aborts to allow core dump generation;
-- otherwise, exit.
function editor_exit (doabort)
  io.stderr:write ("Trying to save modified buffers (if any)...\r\n")

  for _, bp in ipairs (buffers) do
    if bp.modified and not bp.nosave then
      local buf, as = ""
      local i
      local fname = bp.filename or bp.name
      buf = fname .. string.upper (PACKAGE) .. "SAVE"
      io.stderr:write (string.format ("Saving %s...\r\n", buf))
      write_to_disk (bp, buf, "rw-------")
    end
  end

  if doabort then
    posix.abort ()
  else
    posix._exit (2)
  end
end
