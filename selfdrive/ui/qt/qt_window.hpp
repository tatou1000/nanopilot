#include <string>

#include <QWidget>
#include <QApplication>

#ifdef QCOM2
#include <qpa/qplatformnativeinterface.h>
#include <QPlatformSurfaceEvent>
#include <wayland-client-protocol.h>
#endif


#ifdef QCOM2
  const int vwp_w = 2160, vwp_h = 1080;
#else
  const int vwp_w = 1920, vwp_h = 1080;
#endif

inline void setMainWindow(QWidget *w) {
  const float scale = getenv("SCALE") != NULL ? std::stof(getenv("SCALE")) : 1.0;
  w->setFixedSize(vwp_w*scale, vwp_h*scale);
  w->show();

#ifdef QCOM2
  QPlatformNativeInterface *native = QGuiApplication::platformNativeInterface();
  wl_surface *s = reinterpret_cast<wl_surface*>(native->nativeResourceForWindow("surface", w->windowHandle()));
  wl_surface_set_buffer_transform(s, WL_OUTPUT_TRANSFORM_270);
  wl_surface_commit(s);
  w->showFullScreen();
#endif
}
