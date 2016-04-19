from Tkinter import *
import random
import time
#import thread
import threading


def window():
    top = Tk()
    text = Text(top)
    text.insert(INSERT, "The random number is: ")
    text.pack()
    #top.mainloop()

    #x = 1

    #while x < 5:
    #    rand = random.randint(1, 100)
    #    #text.delete(INSERT)
    #    text.insert(END, " " + str(rand))
    #    time.sleep(1)
    #    x += 1
    #    top.update()

    #time.sleep(2)
    text.insert(END, str(4))
    #top.update()
    top.mainloop()


def counter(end):
    x = 1
    while x < end:
        print(str(x))
        x += 1


"""try:
    thread.start_new_thread(counter, (5, ))
    thread.start_new_thread(window, ())
except:
    print("Error: unable to start thread")

while True:
    pass
"""


