$RuOBSD$
--- src/joystick.c.orig	Sun Nov 25 22:55:23 2001
+++ src/joystick.c	Thu Jan 31 18:41:43 2002
@@ -34,7 +34,7 @@
 #if defined(JS_VERSION) && JS_VERSION > 0x010000
 #define NEW_INTERFACE
 #endif
-#ifdef __FreeBSD__
+#if defined(__FreeBSD__) || defined (__OpenBSD__)
 #define JS_DATA_TYPE joystick
 #define JS_RETURN sizeof(struct JS_DATA_TYPE)
 #endif
@@ -56,13 +56,13 @@
 int JoyRead(void){static int res=0,autofire=0,juggle=0;static unsigned int afframe=0;static int juggleval=B_LEFT;static unsigned int juggleframe=0;struct js_event jev;if(read(joystickFd,&jev,sizeof(jev))==sizeof(jev)){if((jev.type&~JS_EVENT_INIT)==JS_EVENT_BUTTON){switch(jev.number){case 0:if(jev.value!=0){res|=B_FIRE;}else{res&=~B_FIRE;}break;case 1:if(jev.value!=0){autofire=res&B_FIRE?0:1;}else{autofire=0;res&=~B_FIRE;}break;case 2:if(jev.value!=0){juggle=(res&(B_LEFT|B_RIGHT))!=0?0:1;}else{juggle=0;res&=~(B_LEFT|B_RIGHT);}break;case 4:if(jev.value!=0){if(autofire!=0){if(CFGGET(joyAutofire)>1){CFGSET(joyAutofire,CFGGET(joyAutofire)- 1);}}else if(juggle!=0){if(CFGGET(joyJuggle)>1){CFGSET(joyJuggle,CFGGET(joyJuggle)- 1);}}else if(CFGGET(fastmode)==0){int speed=CFGGET(speed);if(speed>10){speed-=10;CFGSET(speed,speed);EmulSetSpeed(speed);}}}break;case 5:if(jev.value!=0){if(autofire!=0){if(CFGGET(joyAutofire)<50){CFGSET(joyAutofire,CFGGET(joyAutofire)+1);}}else if(juggle!=0){if(CFGGET(joyJuggle)<50){CFGSET(joyJuggle,CFGGET(joyJuggle)+1);}}else if(CFGGET(fastmode)==0){int speed=CFGGET(speed);if(speed<200){speed+=10;CFGSET(speed,speed);EmulSetSpeed(speed);}}}break;}}else{switch((int)jev.number){case 0:if(jev.value<=-CFGGET(joyTolerance)){res|=B_LEFT;}else if(jev.value>=CFGGET(joyTolerance)){res|=B_RIGHT;}else{res&=~(B_LEFT|B_RIGHT);}break;case 1:if(jev.value<=-CFGGET(joyTolerance)){res|=B_UP;}else if(jev.value>=CFGGET(joyTolerance)){res|=B_DOWN;}else{res&=~(B_UP|B_DOWN);}break;}}}if(autofire!=0){if(50-(EmulVideoFrame-afframe)>(unsigned int)CFGGET(joyAutofire)){afframe=EmulVideoFrame;res^=B_FIRE;}}if(juggle!=0){if(50-(EmulVideoFrame-juggleframe)>(unsigned int)CFGGET(joyJuggle)){juggleframe=EmulVideoFrame;if(res&(B_LEFT|B_RIGHT)){res&=~(B_LEFT|B_RIGHT);}else{res|=juggleval;juggleval^=(B_LEFT|B_RIGHT);}}}return res;}
 #else
 int JoyRead(void){static int afval=0;static unsigned int afframe=0;struct JS_DATA_TYPE jdata;int res=0;if(read(joystickFd,&jdata,JS_RETURN)!=JS_RETURN){Msg(M_PWARN,"lost joystick connection");}else{if(jdata.x<=centreX-CFGGET(joyTolerance)){res|=B_LEFT;}else if(jdata.x>=centreX+CFGGET(joyTolerance)){res|=B_RIGHT;}if(jdata.y<=centreY-CFGGET(joyTolerance)){res|=B_UP;}else if(jdata.y>=centreY+CFGGET(joyTolerance)){res|=B_DOWN;}
-#ifdef __FreeBSD__
+#if defined(__FreeBSD__) || defined(__OpenBSD__)
 if(jdata.b1)
 #else
 if(jdata.buttons&0x01)
 #endif
 {res|=B_FIRE;}
-#ifdef __FreeBSD__
+#if defined(__FreeBSD__) || defined(__OpenBSD__)
 else if(jdata.b2)
 #else
 else if(jdata.buttons&0x02)
