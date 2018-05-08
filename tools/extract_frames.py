#!/usr/bin/env python

"""
extract_frames.py - Extract frames from VID files as PNG files for further
                    processing.

Copyright (C) 2016 David Boddie <david@boddie.org.uk>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
"""

import os, stat, struct, sys, zlib
import Image

class ArgumentError(Exception):
    pass

def find_option(args, label, number = 0):

    """Matches an option in a list of command line arguments, returning a
    single boolean value for options without arguments and a tuple for options
    with arguments.
    
    For options with arguments, the tuple contains a boolean value and a list
    of arguments found unless only one argument is expected, in which case the
    value itself is included in the tuple instead of a list.
    
    If the boolean value is True, the option was found. If it is False then
    either it was not found or the required number of arguments was not found.
    """
    
    try:
        i = args.index(label)
    except ValueError:
        if number == 0:
            return False
        else:
            return False, None
    
    values = args[i + 1:i + number + 1]
    args[:] = args[:i] + args[i + number + 1:]
    
    if number == 0:
        return True
    
    if len(values) < number:
        raise ArgumentError, "Not enough values for argument '%s': %s" % (label, repr(values))
    
    if number == 1:
        values = values[0]
    
    return True, values

def usage():

    sys.stderr.write("Usage: %s [-u] [-d <width>x<height>] [-t <begin>,<end>] <movie data file> <output directory>\n" % sys.argv[0])
    sys.exit(1)


if __name__ == "__main__":

    args = sys.argv[:]
    
    try:
        uncompressed = find_option(args, "-u")
        compressed = not uncompressed
        
        width, height = 512, 384
        d, dim = find_option(args, "-d", 1)
        if d:
            width, height = map(int, dim.split("x"))
    
        first, last = 0, None
        t, span = find_option(args, "-t", 1)
        if t:
            pieces = span.split(",")
            
            if pieces[0]:
                first = int(pieces[0])
            else:
                first = 0
            
            if pieces[1]:
                last = int(pieces[1])
            else:
                last = None
    
    except (ArgumentError, IndexError, ValueError):
        usage()
    
    if len(args) != 3:
        usage()
    
    movie_file = args[-2]
    output_dir = args[-1]
    
    if not os.path.exists(output_dir):
        os.mkdir(output_dir)
    
    if not compressed:
        size = os.stat(sys.argv[1])[stat.ST_SIZE]
        frames = size/(512 * 384)
        field_size = len(str(frames))
        template = "%%0%ii.png" % field_size
    else:
        template = "%i.png"
    
    f = open(movie_file, "rb")
    
    frame = 0
    if width < 512 or height < 384:
        resize_mode = Image.ANTIALIAS
    else:
        resize_mode = Image.NEAREST
    
    
    while True:
    
        if last != None and frame > last:
            break
        
        if compressed:
            size = f.read(4)
            if not size:
                break
            size = struct.unpack("<I", size)[0]
        else:
            size = 512 * 384
        
        data = f.read(size)
        
        if not data or len(data) < size:
            break
        
        # Skip the audio.
        f.seek(625, 1)
        
        if frame < first:
            frame += 1
            continue
        
        if compressed:
            data = zlib.decompress(data)
        
        im = Image.fromstring("P", (512, 384), data)
        im.putpalette("\x00\x00\x00"
                      "\x00\xff\x00"
                      "\xff\xff\x00"
                      "\x00\x00\xff"
                      "\xff\x00\x00"
                      "\xff\xff\xff"
                      "\x00\xff\xff"
                      "\xff\x00\xff")
        im = im.resize((width, height), Image.NEAREST)
        im.save(os.path.join(output_dir, template % frame))
        frame += 1
    
    sys.exit()
