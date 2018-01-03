static void (*openGLUTInit)(int *argcp, char **argv) = NULL;
static void (*openGLUTInitDisplayMode)(unsigned int mode) = NULL;
static void (*openGLUTInitDisplayString)(const char *string) = NULL;
static void (*openGLUTInitWindowPosition)(int x, int y) = NULL;
static void (*openGLUTInitWindowSize)(int width, int height) = NULL;
static void (*openGLUTMainLoop)(void) = NULL;
static int (*openGLUTCreateWindow)(const char *title) = NULL;
static int (*openGLUTCreateSubWindow)(int win, int x, int y, int width, int height) = NULL;
static void (*openGLUTDestroyWindow)(int win) = NULL;
static void (*openGLUTPostRedisplay)(void) = NULL;
static void (*openGLUTPostWindowRedisplay)(int win) = NULL;
static void (*openGLUTSwapBuffers)(void) = NULL;
static int (*openGLUTGetWindow)(void) = NULL;
static void (*openGLUTSetWindow)(int win) = NULL;
static void (*openGLUTSetWindowTitle)(const char *title) = NULL;
static void (*openGLUTSetIconTitle)(const char *title) = NULL;
static void (*openGLUTPositionWindow)(int x, int y) = NULL;
static void (*openGLUTReshapeWindow)(int width, int height) = NULL;
static void (*openGLUTPopWindow)(void) = NULL;
static void (*openGLUTPushWindow)(void) = NULL;
static void (*openGLUTIconifyWindow)(void) = NULL;
static void (*openGLUTShowWindow)(void) = NULL;
static void (*openGLUTHideWindow)(void) = NULL;
static void (*openGLUTFullScreen)(void) = NULL;
static void (*openGLUTSetCursor)(int cursor) = NULL;
static void (*openGLUTWarpPointer)(int x, int y) = NULL;
static void (*openGLUTWMCloseFunc)(void (*func)(void)) = NULL;
static void (*openGLUTCheckLoop)(void) = NULL;
static void (*openGLUTEstablishOverlay)(void) = NULL;
static void (*openGLUTRemoveOverlay)(void) = NULL;
static void (*openGLUTUseLayer)(GLenum layer) = NULL;
static void (*openGLUTPostOverlayRedisplay)(void) = NULL;
static void (*openGLUTPostWindowOverlayRedisplay)(int win) = NULL;
static void (*openGLUTShowOverlay)(void) = NULL;
static void (*openGLUTHideOverlay)(void) = NULL;
static int (*openGLUTCreateMenu)(void (*func)(int)) = NULL;
static void (*openGLUTDestroyMenu)(int menu) = NULL;
static int (*openGLUTGetMenu)(void) = NULL;
static void (*openGLUTSetMenu)(int menu) = NULL;
static void (*openGLUTAddMenuEntry)(const char *label, int value) = NULL;
static void (*openGLUTAddSubMenu)(const char *label, int submenu) = NULL;
static void (*openGLUTChangeToMenuEntry)(int item, const char *label, int value) = NULL;
static void (*openGLUTChangeToSubMenu)(int item, const char *label, int submenu) = NULL;
static void (*openGLUTRemoveMenuItem)(int item) = NULL;
static void (*openGLUTAttachMenu)(int button) = NULL;
static void (*openGLUTDetachMenu)(int button) = NULL;
static void (*openGLUTDisplayFunc)(void (*func)(void)) = NULL;
static void (*openGLUTReshapeFunc)(void (*func)(int width, int height)) = NULL;
static void (*openGLUTKeyboardFunc)(void (*func)(unsigned char key, int x, int y)) = NULL;
static void (*openGLUTMouseFunc)(void (*func)(int button, int state, int x, int y)) = NULL;
static void (*openGLUTMotionFunc)(void (*func)(int x, int y)) = NULL;
static void (*openGLUTPassiveMotionFunc)(void (*func)(int x, int y)) = NULL;
static void (*openGLUTEntryFunc)(void (*func)(int state)) = NULL;
static void (*openGLUTVisibilityFunc)(void (*func)(int state)) = NULL;
static void (*openGLUTIdleFunc)(void (*func)(void)) = NULL;
static void (*openGLUTTimerFunc)(unsigned int millis, void (*func)(int value), int value) = NULL;
static void (*openGLUTMenuStateFunc)(void (*func)(int state)) = NULL;
static void (*openGLUTSpecialFunc)(void (*func)(int key, int x, int y)) = NULL;
static void (*openGLUTSpaceballMotionFunc)(void (*func)(int x, int y, int z)) = NULL;
static void (*openGLUTSpaceballRotateFunc)(void (*func)(int x, int y, int z)) = NULL;
static void (*openGLUTSpaceballButtonFunc)(void (*func)(int button, int state)) = NULL;
static void (*openGLUTButtonBoxFunc)(void (*func)(int button, int state)) = NULL;
static void (*openGLUTDialsFunc)(void (*func)(int dial, int value)) = NULL;
static void (*openGLUTTabletMotionFunc)(void (*func)(int x, int y)) = NULL;
static void (*openGLUTTabletButtonFunc)(void (*func)(int button, int state, int x, int y)) = NULL;
static void (*openGLUTMenuStatusFunc)(void (*func)(int status, int x, int y)) = NULL;
static void (*openGLUTOverlayDisplayFunc)(void (*func)(void)) = NULL;
static void (*openGLUTWindowStatusFunc)(void (*func)(int state)) = NULL;
static void (*openGLUTKeyboardUpFunc)(void (*func)(unsigned char key, int x, int y)) = NULL;
static void (*openGLUTSpecialUpFunc)(void (*func)(int key, int x, int y)) = NULL;
static void (*openGLUTJoystickFunc)(void (*func)(unsigned int buttonMask, int x, int y, int z), int pollInterval) = NULL;
static void (*openGLUTSetColor)(int, GLfloat red, GLfloat green, GLfloat blue) = NULL;
static GLfloat (*openGLUTGetColor)(int ndx, int component) = NULL;
static void (*openGLUTCopyColormap)(int win) = NULL;
static int (*openGLUTGet)(GLenum type) = NULL;
static int (*openGLUTDeviceGet)(GLenum type) = NULL;
static int (*openGLUTExtensionSupported)(const char *name) = NULL;
static int (*openGLUTGetModifiers)(void) = NULL;
static int (*openGLUTLayerGet)(GLenum type) = NULL;
static void * (*openGLUTGetProcAddress)(const char *procName) = NULL;
static void (*openGLUTBitmapCharacter)(void *font, int character) = NULL;
static int (*openGLUTBitmapWidth)(void *font, int character) = NULL;
static void (*openGLUTStrokeCharacter)(void *font, int character) = NULL;
static int (*openGLUTStrokeWidth)(void *font, int character) = NULL;
static int (*openGLUTBitmapLength)(void *font, const unsigned char *string) = NULL;
static int (*openGLUTStrokeLength)(void *font, const unsigned char *string) = NULL;
static void (*openGLUTWireSphere)(GLdouble radius, GLint slices, GLint stacks) = NULL;
static void (*openGLUTSolidSphere)(GLdouble radius, GLint slices, GLint stacks) = NULL;
static void (*openGLUTWireCone)(GLdouble base, GLdouble height, GLint slices, GLint stacks) = NULL;
static void (*openGLUTSolidCone)(GLdouble base, GLdouble height, GLint slices, GLint stacks) = NULL;
static void (*openGLUTWireCube)(GLdouble size) = NULL;
static void (*openGLUTSolidCube)(GLdouble size) = NULL;
static void (*openGLUTWireTorus)(GLdouble innerRadius, GLdouble outerRadius, GLint sides, GLint rings) = NULL;
static void (*openGLUTSolidTorus)(GLdouble innerRadius, GLdouble outerRadius, GLint sides, GLint rings) = NULL;
static void (*openGLUTWireDodecahedron)(void) = NULL;
static void (*openGLUTSolidDodecahedron)(void) = NULL;
static void (*openGLUTWireTeapot)(GLdouble size) = NULL;
static void (*openGLUTSolidTeapot)(GLdouble size) = NULL;
static void (*openGLUTWireOctahedron)(void) = NULL;
static void (*openGLUTSolidOctahedron)(void) = NULL;
static void (*openGLUTWireTetrahedron)(void) = NULL;
static void (*openGLUTSolidTetrahedron)(void) = NULL;
static void (*openGLUTWireIcosahedron)(void) = NULL;
static void (*openGLUTSolidIcosahedron)(void) = NULL;
static int (*openGLUTVideoResizeGet)(GLenum param) = NULL;
static void (*openGLUTSetupVideoResizing)(void) = NULL;
static void (*openGLUTStopVideoResizing)(void) = NULL;
static void (*openGLUTVideoResize)(int x, int y, int width, int height) = NULL;
static void (*openGLUTVideoPan)(int x, int y, int width, int height) = NULL;
static void (*openGLUTReportErrors)(void) = NULL;
static void (*openGLUTIgnoreKeyRepeat)(int ignore) = NULL;
static void (*openGLUTSetKeyRepeat)(int repeatMode) = NULL;
static void (*openGLUTForceJoystickFunc)(void) = NULL;
static void (*openGLUTGameModeString)(const char *string) = NULL;
static int (*openGLUTEnterGameMode)(void) = NULL;
static void (*openGLUTLeaveGameMode)(void) = NULL;
static int (*openGLUTGameModeGet)(GLenum mode) = NULL;
static void
openglut_init(void)
{
	void *handle;
	if (!(handle = dlopen("/System/Library/Frameworks/GLUT.framework/GLUT", RTLD_LAZY | RTLD_LOCAL)))
		err(1, "Could not dlopen /System/Library/Frameworks/GLUT.framework/GLUT");
	if (!(openGLUTInit = dlsym(handle, "glutInit")))
		err(1, "Could not resolve glutInit()");
	if (!(openGLUTInitDisplayMode = dlsym(handle, "glutInitDisplayMode")))
		err(1, "Could not resolve glutInitDisplayMode()");
	if (!(openGLUTInitDisplayString = dlsym(handle, "glutInitDisplayString")))
		err(1, "Could not resolve glutInitDisplayString()");
	if (!(openGLUTInitWindowPosition = dlsym(handle, "glutInitWindowPosition")))
		err(1, "Could not resolve glutInitWindowPosition()");
	if (!(openGLUTInitWindowSize = dlsym(handle, "glutInitWindowSize")))
		err(1, "Could not resolve glutInitWindowSize()");
	if (!(openGLUTMainLoop = dlsym(handle, "glutMainLoop")))
		err(1, "Could not resolve glutMainLoop()");
	if (!(openGLUTCreateWindow = dlsym(handle, "glutCreateWindow")))
		err(1, "Could not resolve glutCreateWindow()");
	if (!(openGLUTCreateSubWindow = dlsym(handle, "glutCreateSubWindow")))
		err(1, "Could not resolve glutCreateSubWindow()");
	if (!(openGLUTDestroyWindow = dlsym(handle, "glutDestroyWindow")))
		err(1, "Could not resolve glutDestroyWindow()");
	if (!(openGLUTPostRedisplay = dlsym(handle, "glutPostRedisplay")))
		err(1, "Could not resolve glutPostRedisplay()");
	if (!(openGLUTPostWindowRedisplay = dlsym(handle, "glutPostWindowRedisplay")))
		err(1, "Could not resolve glutPostWindowRedisplay()");
	if (!(openGLUTSwapBuffers = dlsym(handle, "glutSwapBuffers")))
		err(1, "Could not resolve glutSwapBuffers()");
	if (!(openGLUTGetWindow = dlsym(handle, "glutGetWindow")))
		err(1, "Could not resolve glutGetWindow()");
	if (!(openGLUTSetWindow = dlsym(handle, "glutSetWindow")))
		err(1, "Could not resolve glutSetWindow()");
	if (!(openGLUTSetWindowTitle = dlsym(handle, "glutSetWindowTitle")))
		err(1, "Could not resolve glutSetWindowTitle()");
	if (!(openGLUTSetIconTitle = dlsym(handle, "glutSetIconTitle")))
		err(1, "Could not resolve glutSetIconTitle()");
	if (!(openGLUTPositionWindow = dlsym(handle, "glutPositionWindow")))
		err(1, "Could not resolve glutPositionWindow()");
	if (!(openGLUTReshapeWindow = dlsym(handle, "glutReshapeWindow")))
		err(1, "Could not resolve glutReshapeWindow()");
	if (!(openGLUTPopWindow = dlsym(handle, "glutPopWindow")))
		err(1, "Could not resolve glutPopWindow()");
	if (!(openGLUTPushWindow = dlsym(handle, "glutPushWindow")))
		err(1, "Could not resolve glutPushWindow()");
	if (!(openGLUTIconifyWindow = dlsym(handle, "glutIconifyWindow")))
		err(1, "Could not resolve glutIconifyWindow()");
	if (!(openGLUTShowWindow = dlsym(handle, "glutShowWindow")))
		err(1, "Could not resolve glutShowWindow()");
	if (!(openGLUTHideWindow = dlsym(handle, "glutHideWindow")))
		err(1, "Could not resolve glutHideWindow()");
	if (!(openGLUTFullScreen = dlsym(handle, "glutFullScreen")))
		err(1, "Could not resolve glutFullScreen()");
	if (!(openGLUTSetCursor = dlsym(handle, "glutSetCursor")))
		err(1, "Could not resolve glutSetCursor()");
	if (!(openGLUTWarpPointer = dlsym(handle, "glutWarpPointer")))
		err(1, "Could not resolve glutWarpPointer()");
	if (!(openGLUTWMCloseFunc = dlsym(handle, "glutWMCloseFunc")))
		err(1, "Could not resolve glutWMCloseFunc()");
	if (!(openGLUTCheckLoop = dlsym(handle, "glutCheckLoop")))
		err(1, "Could not resolve glutCheckLoop()");
	if (!(openGLUTEstablishOverlay = dlsym(handle, "glutEstablishOverlay")))
		err(1, "Could not resolve glutEstablishOverlay()");
	if (!(openGLUTRemoveOverlay = dlsym(handle, "glutRemoveOverlay")))
		err(1, "Could not resolve glutRemoveOverlay()");
	if (!(openGLUTUseLayer = dlsym(handle, "glutUseLayer")))
		err(1, "Could not resolve glutUseLayer()");
	if (!(openGLUTPostOverlayRedisplay = dlsym(handle, "glutPostOverlayRedisplay")))
		err(1, "Could not resolve glutPostOverlayRedisplay()");
	if (!(openGLUTPostWindowOverlayRedisplay = dlsym(handle, "glutPostWindowOverlayRedisplay")))
		err(1, "Could not resolve glutPostWindowOverlayRedisplay()");
	if (!(openGLUTShowOverlay = dlsym(handle, "glutShowOverlay")))
		err(1, "Could not resolve glutShowOverlay()");
	if (!(openGLUTHideOverlay = dlsym(handle, "glutHideOverlay")))
		err(1, "Could not resolve glutHideOverlay()");
	if (!(openGLUTCreateMenu = dlsym(handle, "glutCreateMenu")))
		err(1, "Could not resolve glutCreateMenu()");
	if (!(openGLUTDestroyMenu = dlsym(handle, "glutDestroyMenu")))
		err(1, "Could not resolve glutDestroyMenu()");
	if (!(openGLUTGetMenu = dlsym(handle, "glutGetMenu")))
		err(1, "Could not resolve glutGetMenu()");
	if (!(openGLUTSetMenu = dlsym(handle, "glutSetMenu")))
		err(1, "Could not resolve glutSetMenu()");
	if (!(openGLUTAddMenuEntry = dlsym(handle, "glutAddMenuEntry")))
		err(1, "Could not resolve glutAddMenuEntry()");
	if (!(openGLUTAddSubMenu = dlsym(handle, "glutAddSubMenu")))
		err(1, "Could not resolve glutAddSubMenu()");
	if (!(openGLUTChangeToMenuEntry = dlsym(handle, "glutChangeToMenuEntry")))
		err(1, "Could not resolve glutChangeToMenuEntry()");
	if (!(openGLUTChangeToSubMenu = dlsym(handle, "glutChangeToSubMenu")))
		err(1, "Could not resolve glutChangeToSubMenu()");
	if (!(openGLUTRemoveMenuItem = dlsym(handle, "glutRemoveMenuItem")))
		err(1, "Could not resolve glutRemoveMenuItem()");
	if (!(openGLUTAttachMenu = dlsym(handle, "glutAttachMenu")))
		err(1, "Could not resolve glutAttachMenu()");
	if (!(openGLUTDetachMenu = dlsym(handle, "glutDetachMenu")))
		err(1, "Could not resolve glutDetachMenu()");
	if (!(openGLUTDisplayFunc = dlsym(handle, "glutDisplayFunc")))
		err(1, "Could not resolve glutDisplayFunc()");
	if (!(openGLUTReshapeFunc = dlsym(handle, "glutReshapeFunc")))
		err(1, "Could not resolve glutReshapeFunc()");
	if (!(openGLUTKeyboardFunc = dlsym(handle, "glutKeyboardFunc")))
		err(1, "Could not resolve glutKeyboardFunc()");
	if (!(openGLUTMouseFunc = dlsym(handle, "glutMouseFunc")))
		err(1, "Could not resolve glutMouseFunc()");
	if (!(openGLUTMotionFunc = dlsym(handle, "glutMotionFunc")))
		err(1, "Could not resolve glutMotionFunc()");
	if (!(openGLUTPassiveMotionFunc = dlsym(handle, "glutPassiveMotionFunc")))
		err(1, "Could not resolve glutPassiveMotionFunc()");
	if (!(openGLUTEntryFunc = dlsym(handle, "glutEntryFunc")))
		err(1, "Could not resolve glutEntryFunc()");
	if (!(openGLUTVisibilityFunc = dlsym(handle, "glutVisibilityFunc")))
		err(1, "Could not resolve glutVisibilityFunc()");
	if (!(openGLUTIdleFunc = dlsym(handle, "glutIdleFunc")))
		err(1, "Could not resolve glutIdleFunc()");
	if (!(openGLUTTimerFunc = dlsym(handle, "glutTimerFunc")))
		err(1, "Could not resolve glutTimerFunc()");
	if (!(openGLUTMenuStateFunc = dlsym(handle, "glutMenuStateFunc")))
		err(1, "Could not resolve glutMenuStateFunc()");
	if (!(openGLUTSpecialFunc = dlsym(handle, "glutSpecialFunc")))
		err(1, "Could not resolve glutSpecialFunc()");
	if (!(openGLUTSpaceballMotionFunc = dlsym(handle, "glutSpaceballMotionFunc")))
		err(1, "Could not resolve glutSpaceballMotionFunc()");
	if (!(openGLUTSpaceballRotateFunc = dlsym(handle, "glutSpaceballRotateFunc")))
		err(1, "Could not resolve glutSpaceballRotateFunc()");
	if (!(openGLUTSpaceballButtonFunc = dlsym(handle, "glutSpaceballButtonFunc")))
		err(1, "Could not resolve glutSpaceballButtonFunc()");
	if (!(openGLUTButtonBoxFunc = dlsym(handle, "glutButtonBoxFunc")))
		err(1, "Could not resolve glutButtonBoxFunc()");
	if (!(openGLUTDialsFunc = dlsym(handle, "glutDialsFunc")))
		err(1, "Could not resolve glutDialsFunc()");
	if (!(openGLUTTabletMotionFunc = dlsym(handle, "glutTabletMotionFunc")))
		err(1, "Could not resolve glutTabletMotionFunc()");
	if (!(openGLUTTabletButtonFunc = dlsym(handle, "glutTabletButtonFunc")))
		err(1, "Could not resolve glutTabletButtonFunc()");
	if (!(openGLUTMenuStatusFunc = dlsym(handle, "glutMenuStatusFunc")))
		err(1, "Could not resolve glutMenuStatusFunc()");
	if (!(openGLUTOverlayDisplayFunc = dlsym(handle, "glutOverlayDisplayFunc")))
		err(1, "Could not resolve glutOverlayDisplayFunc()");
	if (!(openGLUTWindowStatusFunc = dlsym(handle, "glutWindowStatusFunc")))
		err(1, "Could not resolve glutWindowStatusFunc()");
	if (!(openGLUTKeyboardUpFunc = dlsym(handle, "glutKeyboardUpFunc")))
		err(1, "Could not resolve glutKeyboardUpFunc()");
	if (!(openGLUTSpecialUpFunc = dlsym(handle, "glutSpecialUpFunc")))
		err(1, "Could not resolve glutSpecialUpFunc()");
	if (!(openGLUTJoystickFunc = dlsym(handle, "glutJoystickFunc")))
		err(1, "Could not resolve glutJoystickFunc()");
	if (!(openGLUTSetColor = dlsym(handle, "glutSetColor")))
		err(1, "Could not resolve glutSetColor()");
	if (!(openGLUTGetColor = dlsym(handle, "glutGetColor")))
		err(1, "Could not resolve glutGetColor()");
	if (!(openGLUTCopyColormap = dlsym(handle, "glutCopyColormap")))
		err(1, "Could not resolve glutCopyColormap()");
	if (!(openGLUTGet = dlsym(handle, "glutGet")))
		err(1, "Could not resolve glutGet()");
	if (!(openGLUTDeviceGet = dlsym(handle, "glutDeviceGet")))
		err(1, "Could not resolve glutDeviceGet()");
	if (!(openGLUTExtensionSupported = dlsym(handle, "glutExtensionSupported")))
		err(1, "Could not resolve glutExtensionSupported()");
	if (!(openGLUTGetModifiers = dlsym(handle, "glutGetModifiers")))
		err(1, "Could not resolve glutGetModifiers()");
	if (!(openGLUTLayerGet = dlsym(handle, "glutLayerGet")))
		err(1, "Could not resolve glutLayerGet()");
	if (!(openGLUTGetProcAddress = dlsym(handle, "glutGetProcAddress")))
		err(1, "Could not resolve glutGetProcAddress()");
	if (!(openGLUTBitmapCharacter = dlsym(handle, "glutBitmapCharacter")))
		err(1, "Could not resolve glutBitmapCharacter()");
	if (!(openGLUTBitmapWidth = dlsym(handle, "glutBitmapWidth")))
		err(1, "Could not resolve glutBitmapWidth()");
	if (!(openGLUTStrokeCharacter = dlsym(handle, "glutStrokeCharacter")))
		err(1, "Could not resolve glutStrokeCharacter()");
	if (!(openGLUTStrokeWidth = dlsym(handle, "glutStrokeWidth")))
		err(1, "Could not resolve glutStrokeWidth()");
	if (!(openGLUTBitmapLength = dlsym(handle, "glutBitmapLength")))
		err(1, "Could not resolve glutBitmapLength()");
	if (!(openGLUTStrokeLength = dlsym(handle, "glutStrokeLength")))
		err(1, "Could not resolve glutStrokeLength()");
	if (!(openGLUTWireSphere = dlsym(handle, "glutWireSphere")))
		err(1, "Could not resolve glutWireSphere()");
	if (!(openGLUTSolidSphere = dlsym(handle, "glutSolidSphere")))
		err(1, "Could not resolve glutSolidSphere()");
	if (!(openGLUTWireCone = dlsym(handle, "glutWireCone")))
		err(1, "Could not resolve glutWireCone()");
	if (!(openGLUTSolidCone = dlsym(handle, "glutSolidCone")))
		err(1, "Could not resolve glutSolidCone()");
	if (!(openGLUTWireCube = dlsym(handle, "glutWireCube")))
		err(1, "Could not resolve glutWireCube()");
	if (!(openGLUTSolidCube = dlsym(handle, "glutSolidCube")))
		err(1, "Could not resolve glutSolidCube()");
	if (!(openGLUTWireTorus = dlsym(handle, "glutWireTorus")))
		err(1, "Could not resolve glutWireTorus()");
	if (!(openGLUTSolidTorus = dlsym(handle, "glutSolidTorus")))
		err(1, "Could not resolve glutSolidTorus()");
	if (!(openGLUTWireDodecahedron = dlsym(handle, "glutWireDodecahedron")))
		err(1, "Could not resolve glutWireDodecahedron()");
	if (!(openGLUTSolidDodecahedron = dlsym(handle, "glutSolidDodecahedron")))
		err(1, "Could not resolve glutSolidDodecahedron()");
	if (!(openGLUTWireTeapot = dlsym(handle, "glutWireTeapot")))
		err(1, "Could not resolve glutWireTeapot()");
	if (!(openGLUTSolidTeapot = dlsym(handle, "glutSolidTeapot")))
		err(1, "Could not resolve glutSolidTeapot()");
	if (!(openGLUTWireOctahedron = dlsym(handle, "glutWireOctahedron")))
		err(1, "Could not resolve glutWireOctahedron()");
	if (!(openGLUTSolidOctahedron = dlsym(handle, "glutSolidOctahedron")))
		err(1, "Could not resolve glutSolidOctahedron()");
	if (!(openGLUTWireTetrahedron = dlsym(handle, "glutWireTetrahedron")))
		err(1, "Could not resolve glutWireTetrahedron()");
	if (!(openGLUTSolidTetrahedron = dlsym(handle, "glutSolidTetrahedron")))
		err(1, "Could not resolve glutSolidTetrahedron()");
	if (!(openGLUTWireIcosahedron = dlsym(handle, "glutWireIcosahedron")))
		err(1, "Could not resolve glutWireIcosahedron()");
	if (!(openGLUTSolidIcosahedron = dlsym(handle, "glutSolidIcosahedron")))
		err(1, "Could not resolve glutSolidIcosahedron()");
	if (!(openGLUTVideoResizeGet = dlsym(handle, "glutVideoResizeGet")))
		err(1, "Could not resolve glutVideoResizeGet()");
	if (!(openGLUTSetupVideoResizing = dlsym(handle, "glutSetupVideoResizing")))
		err(1, "Could not resolve glutSetupVideoResizing()");
	if (!(openGLUTStopVideoResizing = dlsym(handle, "glutStopVideoResizing")))
		err(1, "Could not resolve glutStopVideoResizing()");
	if (!(openGLUTVideoResize = dlsym(handle, "glutVideoResize")))
		err(1, "Could not resolve glutVideoResize()");
	if (!(openGLUTVideoPan = dlsym(handle, "glutVideoPan")))
		err(1, "Could not resolve glutVideoPan()");
	if (!(openGLUTReportErrors = dlsym(handle, "glutReportErrors")))
		err(1, "Could not resolve glutReportErrors()");
	if (!(openGLUTIgnoreKeyRepeat = dlsym(handle, "glutIgnoreKeyRepeat")))
		err(1, "Could not resolve glutIgnoreKeyRepeat()");
	if (!(openGLUTSetKeyRepeat = dlsym(handle, "glutSetKeyRepeat")))
		err(1, "Could not resolve glutSetKeyRepeat()");
	if (!(openGLUTForceJoystickFunc = dlsym(handle, "glutForceJoystickFunc")))
		err(1, "Could not resolve glutForceJoystickFunc()");
	if (!(openGLUTGameModeString = dlsym(handle, "glutGameModeString")))
		err(1, "Could not resolve glutGameModeString()");
	if (!(openGLUTEnterGameMode = dlsym(handle, "glutEnterGameMode")))
		err(1, "Could not resolve glutEnterGameMode()");
	if (!(openGLUTLeaveGameMode = dlsym(handle, "glutLeaveGameMode")))
		err(1, "Could not resolve glutLeaveGameMode()");
	if (!(openGLUTGameModeGet = dlsym(handle, "glutGameModeGet")))
		err(1, "Could not resolve glutGameModeGet()");
	if (!(glutStrokeRoman = dlsym(handle, "glutStrokeRoman")))
		err(1, "Could not resolve glutStrokeRoman()");
}
void
glutInitDisplayMode(unsigned int mode)
{
	openGLUTInitDisplayMode(mode);
}
int
glutCreateMenu(void (*func)(int))
{
	return openGLUTCreateMenu(func);
}
void
glutAddMenuEntry(const char *label, int value)
{
	openGLUTAddMenuEntry(label, value);
}
void
glutAddSubMenu(const char *label, int submenu)
{
	openGLUTAddSubMenu(label, submenu);
}
void
glutChangeToMenuEntry(int item, const char *label, int value)
{
	openGLUTChangeToMenuEntry(item, label, value);
}
void
glutAttachMenu(int button)
{
	openGLUTAttachMenu(button);
}
void
glutDisplayFunc(void (*func)(void))
{
	openGLUTDisplayFunc(func);
}
void
glutKeyboardFunc(void (*func)(unsigned char key, int x, int y))
{
	openGLUTKeyboardFunc(func);
}
void
glutMouseFunc(void (*func)(int button, int state, int x, int y))
{
	openGLUTMouseFunc(func);
}
void
glutMotionFunc(void (*func)(int x, int y))
{
	openGLUTMotionFunc(func);
}
void
glutVisibilityFunc(void (*func)(int state))
{
	openGLUTVisibilityFunc(func);
}
void
glutStrokeCharacter(void *font, int character)
{
	openGLUTStrokeCharacter(font, character);
}
void
glutSolidSphere(GLdouble radius, GLint slices, GLint stacks)
{
	openGLUTSolidSphere(radius, slices, stacks);
}
void
glutWireCube(GLdouble size)
{
	openGLUTWireCube(size);
}
void
glutSolidTorus(GLdouble innerRadius, GLdouble outerRadius, GLint sides, GLint rings)
{
	openGLUTSolidTorus(innerRadius, outerRadius, sides, rings);
}
void
glutSolidDodecahedron(void)
{
	openGLUTSolidDodecahedron();
}
void
glutSolidTeapot(GLdouble size)
{
	openGLUTSolidTeapot(size);
}
void
glutSolidTetrahedron(void)
{
	openGLUTSolidTetrahedron();
}
void
glutSolidIcosahedron(void)
{
	openGLUTSolidIcosahedron();
}
