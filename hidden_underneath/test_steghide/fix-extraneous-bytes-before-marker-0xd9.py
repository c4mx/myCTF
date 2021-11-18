from __future__ import print_function, unicode_literals

import binascii
import os
import sys
import time
import re
import cv2

class capture:
    def __init__(self, what):
        self.what = what
    def __enter__(self):
        self.r, w = os.pipe()
        self.original = os.dup(self.what.fileno())  # save old file descriptor
        self.what.flush()               # flush cache before replacing
        os.dup2(w, self.what.fileno())  # overwrite with pipe
        os.write(w, ' ') # so that subsequent read does not block
        os.close(w)
        return self
    def __exit__(self, exc_type, exc_val, exc_tb):
        self.what.flush() # flush again before reading and restoring
        self.data = os.read(self.r, 1000).strip()
        os.dup2(self.original, self.what.fileno())  # restore original
        os.close(self.r)
    def __str__(self):
        return self.data

def d(chunk):
    print(len(chunk), binascii.hexlify(chunk))

def main(filepath):
    print('#'*80)

    with capture(sys.stderr) as c:
        cv2.imread(filepath)

    ending = "extraneous bytes before marker 0xd9"
    if not str(c).endswith(ending):
        print("File ok.")
        return

    print("Problematic file detected. Message: '{}'".format(c))
    size = int(re.search(b'Corrupt JPEG data: ([\d]+)', str(c)).group(1))

    with file(filepath, 'rb') as f:
        data = f.read()

    data = re.split(b'\xff(?!\x00)', data)

    assert data[-1][0] == b'\xd9' # end of file marker
    data[-1] = data[-1][0] # remove everything after the marker

    assert data[-2][0] == b'\xd7' # if last segment is marked with 0xD7, remove it
    assert len(data[-2]) - 1 == size # check it is the expected size
    print("Removing {} bytes.".format(size))
    del data[-2]

    with open(filepath+'.jpg', 'wb') as f:
        print("Creating new file: {}".format(f.name))
        f.write(b'\xff'.join(data))

    print("Verifying that reading the new file returns no error...")
    with capture(sys.stderr) as c:
        cv2.imread(f.name)
    assert str(c) == ''
    print("Done.")

if __name__ == "__main__":
    main(sys.argv[1])
