#include <X11/Xlib.h>
#include <X11/XF86keysym.h>
#include <X11/keysym.h>
#include <X11/XKBlib.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include "sowm.h"
static client       *list = {0}, *ws_list[10] = {0}, *cur;
static int          ws = 1, sw, sh, wx, wy, numlock = 0;
static unsigned int ww, wh;
static Display      *d;
static XButtonEvent mouse;
static Window       root;
static void (*events[LASTEvent])(XEvent *e) = {
[ButtonPress]      = button_press,
[ButtonRelease]    = button_release,
[ConfigureRequest] = configure_request,
[KeyPress]         = key_press,
[MapRequest]       = map_request,
[MappingNotify]    = mapping_notify,
[EnterNotify]      = notify_enter,
[MotionNotify]     = notify_motion
};
#include "config.h"
void win_focus(client *c) {
cur = c;
XSetInputFocus(d, cur->w, RevertToParent, CurrentTime);
}
void notify_enter(XEvent *e) {
while(XCheckTypedEvent(d, EnterNotify, e));
for win if (c->w == e->xcrossing.window) win_focus(c);
}
void notify_motion(XEvent *e) {
if (!mouse.subwindow || cur->f) return;
while(XCheckTypedEvent(d, MotionNotify, e));
int xd = e->xbutton.x_root - mouse.x_root;
int yd = e->xbutton.y_root - mouse.y_root;
XMoveResizeWindow(d, mouse.subwindow,
wx + (mouse.button == 1 ? xd : 0),
wy + (mouse.button == 1 ? yd : 0),
MAX(1, ww + (mouse.button == 3 ? xd : 0)),
MAX(1, wh + (mouse.button == 3 ? yd : 0)));
}

void key_press(XEvent *e) {
KeySym keysym = XkbKeycodeToKeysym(d, e->xkey.keycode, 0, 0);

for (unsigned int i=0; i < sizeof(keys)/sizeof(*keys); ++i)
if (keys[i].keysym == keysym &&
    mod_clean(keys[i].mod) == mod_clean(e->xkey.state))
    keys[i].function(keys[i].arg);
}

void button_press(XEvent *e) {
if (!e->xbutton.subwindow) return;

win_size(e->xbutton.subwindow, &wx, &wy, &ww, &wh);
XRaiseWindow(d, e->xbutton.subwindow);
mouse = e->xbutton;
}

void button_release(XEvent *e) {
mouse.subwindow = 0;
}

void win_add(Window w) {
client *c;

if (!(c = (client *) calloc(1, sizeof(client))))
exit(1);

c->w = w;

if (list) {
list->prev->next = c;
c->prev          = list->prev;
list->prev       = c;
c->next          = list;

} else {
list = c;
list->prev = list->next = list;
}

ws_save(ws);
}


void win_kill(const Arg arg) {
if (cur) XKillClient(d, cur->w);
}

void configure_request(XEvent *e) {
    XConfigureRequestEvent *ev = &e->xconfigurerequest;

    XConfigureWindow(d, ev->window, ev->value_mask, &(XWindowChanges) {
        .x          = ev->x,
        .y          = ev->y,
        .width      = ev->width,
        .height     = ev->height,
        .sibling    = ev->above,
        .stack_mode = ev->detail
    });
}

void map_request(XEvent *e) {
    Window w = e->xmaprequest.window;

    XSelectInput(d, w, StructureNotifyMask|EnterWindowMask);
    win_size(w, &wx, &wy, &ww, &wh);
    win_add(w);
    cur = list->prev;


    XMapWindow(d, w);
    win_focus(list->prev);
}

void mapping_notify(XEvent *e) {
    XMappingEvent *ev = &e->xmapping;

    if (ev->request == MappingKeyboard || ev->request == MappingModifier) {
        XRefreshKeyboardMapping(ev);
        input_grab(root);
    }
}

void run(const Arg arg) {
    if (fork()) return;
    if (d) close(ConnectionNumber(d));

    setsid();
    execvp((char*)arg.com[0], (char**)arg.com);
}

void input_grab(Window root) {
    unsigned int i, j, modifiers[] = {0, LockMask, numlock, numlock|LockMask};
    XModifierKeymap *modmap = XGetModifierMapping(d);
    KeyCode code;

    for (i = 0; i < 8; i++)
        for (int k = 0; k < modmap->max_keypermod; k++)
            if (modmap->modifiermap[i * modmap->max_keypermod + k]
                == XKeysymToKeycode(d, 0xff7f))
                numlock = (1 << i);

    XUngrabKey(d, AnyKey, AnyModifier, root);
    for (i = 0; i < sizeof(keys)/sizeof(*keys); i++)
        if ((code = XKeysymToKeycode(d, keys[i].keysym)))
            for (j = 0; j < sizeof(modifiers)/sizeof(*modifiers); j++)
                XGrabKey(d, code, keys[i].mod | modifiers[j], root,
                        True, GrabModeAsync, GrabModeAsync);
    for (i = 1; i < 4; i += 2)
        for (j = 0; j < sizeof(modifiers)/sizeof(*modifiers); j++)
            XGrabButton(d, i, MOD | modifiers[j], root, True,
                ButtonPressMask|ButtonReleaseMask|PointerMotionMask,
                GrabModeAsync, GrabModeAsync, 0, 0);
    XFreeModifiermap(modmap);
}
int main(void) {
    XEvent ev;
    if (!(d = XOpenDisplay(0))) exit(1);
    signal(SIGCHLD, SIG_IGN);
    XSetErrorHandler(xerror);
    int s = DefaultScreen(d);
    root  = RootWindow(d, s);
    sw    = XDisplayWidth(d, s);
    sh    = XDisplayHeight(d, s);
    XSelectInput(d,  root, SubstructureRedirectMask);
    XDefineCursor(d, root, XCreateFontCursor(d, 68));
    input_grab(root);
    while (1 && !XNextEvent(d, &ev)) // 1 && will forever be here.
        if (events[ev.type]) events[ev.type](&ev);
}

