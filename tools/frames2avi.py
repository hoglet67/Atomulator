#!/usr/bin/env python

"""
frames2avi.py - Convert VID files containing video frames and audio samples
                into AVI files for further processing.

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

import commands, os, stat, struct, sys, zlib
import Image

class AVIWriter:

    def __init__(self, file_name):
    
        self.f = open(file_name, "wb")
        self.file_name = file_name
        self.addresses = []
    
    def write_int(self, value):
    
        self.f.write(struct.pack("<I", value))
    
    def write_str(self, text):
    
        self.f.write(text)
    
    def begin_header(self):
    
        self.f.write("RIFF")
        self.f.write(struct.pack("<I", 0))
        
        self.f.write("AVI ")
    
    def begin_list(self, name):
    
        self.f.write("LIST")
        self.addresses.append(self.f.tell())
        self.write_int(0)
        self.f.write(name)
    
    def end_list(self):
    
        self.end_chunk()
    
    def begin_chunk(self, name):
    
        self.f.write(name)
        self.addresses.append(self.f.tell())
        self.write_int(0)
    
    def end_chunk(self):
    
        address = self.f.tell()
        length_field_address = self.addresses.pop()
        self.f.seek(length_field_address)
        self.write_int(address - length_field_address - 4)
        self.f.seek(address)
        
        # Apparently we have to ensure that each chunk is padded out to a two-
        # byte boundary.
        while self.f.tell() % 2 != 0:
            self.f.write("\x00")
    
    def end_header(self):
    
        offset = self.f.tell() % 0x800
        
        while offset != 0:
        
            remaining = 2048 - offset
            if remaining >= 8:
                self.begin_chunk("JUNK")
                self.f.write("\x00" * (remaining - 8))
                self.end_chunk()
            
            offset = self.f.tell() % 0x800
    
    def close(self):
    
        length = self.f.tell()
        self.f.seek(4)
        self.write_int(length)
        self.f.close()


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

    sys.stderr.write("Usage: %s [-u] [-s] [-d <width>x<height>] [-t <begin>,<end>] <movie data file> <AVI file>\n" % sys.argv[0])
    sys.stderr.write("The time span is specified as [first],[last] in frames.\n")
    sys.exit(1)


if __name__ == "__main__":

    args = sys.argv[:]
    
    try:
        uncompressed = find_option(args, "-u")
        compressed = not uncompressed
        
        allow_non_integer_scaling = find_option(args, "-s")
        
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
    avi_file = args[-1]
    
    if os.path.abspath(movie_file) == os.path.abspath(avi_file):
        sys.stderr.write("Cannot overwrite the movie data file while reading it.\n")
        sys.exit(1)
    
    placeholders = []
    
    length = os.stat(movie_file)[stat.ST_SIZE]
    
    f = open(movie_file, "rb")
    avi = AVIWriter(avi_file)
    
    avi.begin_header()
    avi.begin_list("hdrl")      # hdrl
    
    avi.begin_chunk("avih")       # avih
    avi.write_int(int(1e6/60))      # time per frame (in microseconds)
    avi.write_int(0)
    avi.write_int(0)
    avi.write_int(0)
    placeholders.append(avi.f.tell())
    avi.write_int(0)                # number of frames
    avi.write_int(0)                # initial frame
    avi.write_int(2)                # number of streams
    avi.write_int(0)
    avi.write_int(width)            # width
    avi.write_int(height)           # height
    avi.write_int(0)
    avi.write_int(0)
    avi.write_int(0)
    avi.write_int(0)
    avi.end_chunk()
    
    avi.begin_list("strl")        # strl
    avi.begin_chunk("strh")         # strh
    avi.write_str("vids")             # (stream type)
    avi.write_str("MPNG")             # (stream handler)
    #avi.write_str("bmp ")            # (stream handler)
    avi.write_int(0)                  # flags
    avi.write_int(0)                  # priority and language
    avi.write_int(0)                  # initial frame
    avi.write_int(1)                  # time scale
    avi.write_int(60)                 # frames per second
    avi.write_int(0)                  # starting time
    placeholders.append(avi.f.tell())
    avi.write_int(0)                  # length
    avi.write_int(0)                  # suggested buffer size
    avi.write_int(0)                  # quality
    avi.write_int(0)                  # sample size (variable)
    avi.write_int(0)                  # frame
    avi.write_int(0)                  # frame
    avi.end_chunk()
    avi.begin_chunk("strf")         # strf
    avi.write_int(0)
    avi.write_int(0)
    avi.write_int(0)
    avi.write_int(0x00180001)       # 1 bit plane, 24 bits per pixel
    avi.write_str("MPNG")
    #avi.write_str("bmp ")
    avi.write_int(0)
    avi.write_int(0)
    avi.write_int(0)
    avi.write_int(0)
    avi.write_int(0)
    avi.end_chunk()                 #
    avi.end_list()                # strl
    
    #avi.begin_list("strl")        # strl
    #avi.begin_chunk("strh")         # strh
    #avi.write_str("auds")             # (stream type)
    #avi.write_int(0)
    #avi.write_int(0)                  # flags
    #avi.write_int(0)                  # priority and language
    #avi.write_int(0)                  # initial frame
    #avi.write_int(1)                  # time scale
    #avi.write_int(60)                 # frames per second
    #avi.write_int(0)                  # starting time
    #placeholders.append(avi.f.tell())
    #avi.write_int(0)                  # length
    #avi.write_int(0)                  # suggested buffer size
    #avi.write_int(0)                  # quality
    #avi.write_int(0)                  # sample size (variable)
    #avi.write_int(0)                  # frame
    #avi.write_int(0)                  # frame
    #avi.end_chunk()
    #avi.begin_chunk("strf")         # strf
    #avi.write_int(0x00010001)         # format (PCM), channels
    #avi.write_int(31250)
    #avi.write_int(31250)
    #avi.write_int(0x00100002)         # block align (channels * bits)/8 (2), bits (16)
    #avi.end_chunk()
    #avi.end_list()                # strl

    avi.end_list()              # hdrl
    avi.end_header()
    
    avi.begin_list("movi")      # movi
    
    frame = 0
    if width < 512 or height < 384:
        resize_mode = Image.ANTIALIAS
    else:
        resize_mode = Image.NEAREST
    
    # Calculate the size of the image within the target size.
    image_width = 512
    image_height = 384
    
    if width != 512 or height != 384:
    
        xscale = width/512.0
        yscale = height/384.0
        scale = min(xscale, yscale)
        
        if allow_non_integer_scaling or int(scale) == scale:
            image_width = int(scale * 512)
            image_height = int(scale * 384)
    else:
        image_width = width
        image_height = height
    
    print "Generating images of size %i x %i within frames of size %i x %i." % (
        image_width, image_height, width, height)
    
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
        
        if frame < first:
            f.seek(625, 1)
            frame += 1
            continue
        
        if compressed:
            data = zlib.decompress(data)
        
        avi.begin_list("rec ")    # rec
        
        im = Image.fromstring("P", (512, 384), data)
        
        # Resize the image if its size does not match the target image size.
        if image_width != 512 or image_height != 384:
            im = im.resize((image_width, image_height), resize_mode)
        
        # Pad the image so that it fits within the target image.
        if image_width != width or image_height != height:
            target = Image.new("P", (width, height), 0)
            target.paste(im, ((width - image_width)/2, (height - image_height)/2))
            im = target
        
        im.putpalette("\x00\x00\x00"
                      "\x00\xff\x00"
                      "\xff\xff\x00"
                      "\x00\x00\xff"
                      "\xff\x00\x00"
                      "\xff\xff\xff"
                      "\x00\xff\xff"
                      "\xff\x00\xff")
        
        # Read and discard invalid audio data for now.
        #avi.begin_chunk("01wb")     # 01wb
        audio_data = f.read(625)
        #audio_data = audio_data.replace("\x7f", "\x61\x0f").replace("\x00", "\x00\x00")
        #avi.f.write(audio_data)
        #avi.end_chunk()
        
        avi.begin_chunk("00dc")     # 00dc
        #im.save(avi.f, "BMP")
        im.save(avi.f, "PNG")
        avi.end_chunk()             # 00dc
        
        avi.end_list()
        
        frame += 1
        elapsed = (frame - first)/60.0
        percent = (100.0 * f.tell())/length
 
        if last != None:
            sys.stdout.write("\rEncoded %i of %i (%1.2f seconds)" % (
                frame - first, last - first, elapsed))
        else:
            sys.stdout.write("\rEncoded %i (%1.2f seconds, %1.2f%%)" % (
                frame - first, elapsed, percent))
    
    avi.end_list()              # movi
    
    frames = frame - first + 1
    for p in placeholders:
        avi.f.seek(p)
        avi.write_int(frames)
    
    avi.close()
    print
    print "Ways to process the output file:"
    print
    #print "mencoder %s -o %s -ovc lavc -oac lavc -lavcopts vcodec=mpeg4:acodec=libmp3lame -srate 44100" % (avi_file, avi_file.replace(".avi", ".mp4"))
    avi_file = commands.mkarg(avi_file)
    wav_file = avi_file.replace(".avi", ".wav")
    wav_44100_file = wav_file.replace(".wav", "-44100.wav")
    mp4_file = avi_file.replace(".avi", ".mp4")
    print "ffmpeg -i %s -map 0:1 %s" % (avi_file, wav_file)
    print "sox %s %s rate 44100" % (wav_file, wav_44100_file)
    print "ffmpeg -i %s -i %s -map 0:0 -map 1:0 -strict experimental %s" % (avi_file, wav_44100_file, mp4_file)
    print
    print "ffmpeg -i %s %s" % (avi_file, avi_file.replace(".avi", ".webm"))
    
    sys.exit()
