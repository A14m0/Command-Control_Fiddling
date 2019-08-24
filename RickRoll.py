import os
import sys
import subprocess
import time
import pythoncom, pyHook
from pynput.keyboard import Key, Controller
from ctypes import *
keyboard = Controller()

filename = "/Users/fiona/Desktop/NickThing/troll.mp4" #This is for macos testing
#Change the file path accordingly

class Video(object):
    def __init__(self,path):
        self.path = path

    def play(self):
    	if sys.platform == "win32":
       		os.startfile(filename)
    	else:
        	opener ="open" if sys.platform == "darwin" else "xdg-open"
        	subprocess.call([opener, filename])
        	if sys.platform == "darwin":
        		# Weird ass muthafuckin shit permissions that have to be granted on mac. 
        		# https://stackoverflow.com/questions/7529991/disable-or-lock-mouse-and-keyboard-in-python
        		# Above link for windows.
				# 
        		# This shit right here just fullscreens and plays it
        		time.sleep(1.5)
        		keyboard.press(Key.cmd)
        		keyboard.press(Key.ctrl_l)
        		keyboard.press('f')
        		keyboard.release(Key.cmd)
        		keyboard.release(Key.ctrl_l)
        		keyboard.release('f')
        		time.sleep(1)
        		keyboard.press(Key.space)
        		keyboard.release(Key.space)
        	if sys.platform == "win32":
        		##
        		## Full screen it and play it. 
        		## Also if possible set volume to 100.
        		##
				ok = windll.user32.BlockInput(True) #enable block on user controls >:)
				time.sleep(215) # 3 min 35 sec, "never gonna give you up" is 3 min 30 sec.
				ok = windll.user32.BlockInput(False) #disable block on user controls :(

class Movie_MP4(Video):
    type = "MP4"

movie = Movie_MP4(r"C:\My Documents\My Videos\Heres_a_file.mp4")
if input("Press enter to play, anything else to exit") == '':
    movie.play()