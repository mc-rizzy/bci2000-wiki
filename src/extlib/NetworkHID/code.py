print("Hello World!")

import usb_hid
from adafruit_hid.keyboard import Keyboard
from adafruit_hid.keyboard_layout_us import KeyboardLayoutUS
from adafruit_hid.keycode import Keycode
from adafruit_hid.mouse import Mouse

import time

import wifi

import struct
import socketpool

ap_ssid = "pico"
ap_password = "pass12345"

recv_host = str(wifi.radio.ipv4_address_ap)
recv_port = 1024


magic = 0x0bc12000
# Each message is formatted as such:
# [0-3] 0x0bc12000
# [4-7] message type
# [8-11] message argument 1
# [12-15] message argument 2
# 
# Message Types:
# 1: Keyboard Press
# 2: Keyboard Release
# 3: Mouse Movement
# 4: Mouse Button


MessageType = {
    "KBD_PRESS": 1,
    "KBD_RELEASE": 2,
    "MOUSE_MV": 3,
    "MOUSE_BUTTON": 4
    }

struct_fmt = ">IIii"
struct_size = struct.calcsize(struct_fmt)

# number of messages held in buffer
buf_len = 16
buf_size = buf_len*struct_size
in_buffer = bytearray(buf_size)

print("aaa")

wifi.radio.start_ap(ssid=ap_ssid, password=ap_password)

print("Access point {} with password {}, this device has ip {}".format(ap_ssid, ap_password, wifi.radio.ipv4_address_ap))

pool = socketpool.SocketPool(wifi.radio)

import socketpool

recv_host = str(wifi.radio.ipv4_address_ap)
sock = pool.socket(pool.AF_INET, pool.SOCK_STREAM)

print("binding", (recv_host, recv_port))

sock.bind((recv_host, recv_port))
backlog = 2
timeout = None
sock.listen(backlog)

kbd = Keyboard(usb_hid.devices)
layout = KeyboardLayoutUS(kbd)
mouse = Mouse(usb_hid.devices)

def clamp(n, min, max):
    if n > max:
        return max
    elif n < min:
        return min
    return n

while True:
    print("Waiting for connection from client")
    conn, addr = sock.accept()
    conn.settimeout(timeout)
    print("Connected from {}".format(addr))
    
    while True:
        sz = conn.recv_into(in_buffer, buf_size)
        print("recv ", sz, "b")
        if sz == 0:
            conn.close()
            break
        if sz % struct_size != 0:
            print("malformed message received with size ", sz, ", expected a multiple of ", struct_size)
        n_recv = sz // struct_size
        print("received {} messages".format(n_recv))
        for i in range(n_recv):
            msg = struct.unpack_from(struct_fmt, in_buffer, i*struct_size)
            msg_magic = msg[0]
            if msg_magic != magic:
                print("Received malformed messsage ", msg)
                continue
            msgtype = msg[1]
            if msgtype == MessageType["KBD_PRESS"]:
                key_ascii = msg[2]
                print("kbd press key ", key_ascii)
                keycodes = layout.keycodes(chr(key_ascii))
                for code in keycodes:
                    kbd.press(code)
            elif msgtype == MessageType["KBD_RELEASE"]:
                key_ascii = msg[2]
                print("kbd release keycode ", key_ascii)
                keycodes = layout.keycodes(chr(key_ascii))
                for code in keycodes:
                    kbd.release(code)
            elif msgtype == MessageType["MOUSE_MV"]:
                # constrain values because big numbers cause the device to die for some reason
                move_x = clamp(msg[2], -100, 100)
                move_y = clamp(msg[3], -100, 100)
                print("move mouse x: ", move_x, " y: ", move_y)
                mouse.move(move_x, move_y, 0)
            elif msgtype ==  MessageType["MOUSE_BUTTON"]:
                mouse1 = msg[2] & 1
                mouse2 = msg[3] & 1
                mpress = 0
                mrelease = 0
                if mouse1:
                    mpress |= Mouse.LEFT_BUTTON
                else:
                    mrelease |= Mouse.LEFT_BUTTON
                if mouse2:
                    mpress |= Mouse.RIGHT_BUTTON
                else:
                    mrelease |= Mouse.RIGHT_BUTTON
                print("mouse button left ", mouse1, " right ", mouse2);
                if mpress:
                    mouse.press(mpress)
                if mrelease:
                    mouse.release(mrelease)
            else:
                print("Recv invalid message type ", msgtype)
                continue
