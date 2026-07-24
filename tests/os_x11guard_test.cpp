#include <tools/os.h>

#include <gtest/gtest.h>

// Regression guard for the Wayland / non-X11 null-Display crash.
//
// The unit-test process runs under the offscreen QPA platform (see tests/main.cpp),
// so there is no xcb connection and os's internal x11Display() returns null — the
// same state a native Wayland session presents. The X11-only window-picker helpers
// must degrade to None (0) instead of dereferencing the null Display*:
// DefaultRootWindow() is a macro that dereferences its argument (an instant segfault
// on nullptr in os::windowUnderCursor), and the Xlib calls in os::findRealWindow
// crash on a null Display. Before the guards these calls segfaulted; with them they
// return None.
#if defined(Q_OS_LINUX)
TEST(OsX11Guard, WindowUnderCursorDegradesToNoneWithoutX11)
{
    EXPECT_EQ(os::windowUnderCursor(true), static_cast<Window>(0));
    EXPECT_EQ(os::windowUnderCursor(false), static_cast<Window>(0));
}

TEST(OsX11Guard, FindRealWindowDegradesToNoneWithoutX11)
{
    EXPECT_EQ(os::findRealWindow(0), static_cast<Window>(0));
}
#endif
