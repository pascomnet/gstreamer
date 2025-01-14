
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "gsthlselements.h"

GST_DEBUG_CATEGORY (hls_debug);

void
hls_element_init (void)
{
  static gsize res = FALSE;
  if (g_once_init_enter (&res)) {
    GST_DEBUG_CATEGORY_INIT (hls_debug, "hlsng", 0,
        "HTTP Live Streaming (HLS) NG");
    g_once_init_leave (&res, TRUE);
  }
}
