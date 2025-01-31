/* GStreamer
 * Copyright (C) 2010 Marc-Andre Lureau <marcandre.lureau@gmail.com>
 * Copyright (C) 2010 Andoni Morales Alastruey <ylatuya@gmail.com>
 * Copyright (C) 2015 Tim-Philipp Müller <tim@centricular.com>
 *
 * Copyright (C) 2021-2022 Centricular Ltd
 *   Author: Edward Hervey <edward@centricular.com>
 *   Author: Jan Schmidt <jan@centricular.com>
 *
 * m3u8.h:
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef __M3U8_H__
#define __M3U8_H__

#include <gst/gst.h>

G_BEGIN_DECLS

typedef struct _GstHLSMediaPlaylist GstHLSMediaPlaylist;
typedef struct _GstHLSTimeMap GstHLSTimeMap;
typedef struct _GstM3U8MediaSegment GstM3U8MediaSegment;
typedef struct _GstM3U8InitFile GstM3U8InitFile;
typedef struct _GstHLSRenditionStream GstHLSRenditionStream;
typedef struct _GstM3U8Client GstM3U8Client;
typedef struct _GstHLSVariantStream GstHLSVariantStream;
typedef struct _GstHLSMasterPlaylist GstHLSMasterPlaylist;

#define GST_HLS_MEDIA_PLAYLIST(m) ((GstHLSMediaPlaylist*)m)
#define GST_M3U8_MEDIA_SEGMENT(f) ((GstM3U8MediaSegment*)f)

#define GST_HLS_MEDIA_PLAYLIST_LOCK(m) g_mutex_lock (&m->lock);
#define GST_HLS_MEDIA_PLAYLIST_UNLOCK(m) g_mutex_unlock (&m->lock);

#define GST_HLS_MEDIA_PLAYLIST_IS_LIVE(m) ((m)->endlist == FALSE)

/* hlsdemux must not get closer to the end of a live stream than
   GST_M3U8_LIVE_MIN_FRAGMENT_DISTANCE fragments. Section 6.3.3
   "Playing the Playlist file" of the HLS draft states that this
   value is three fragments */
#define GST_M3U8_LIVE_MIN_FRAGMENT_DISTANCE 3

typedef enum {
  GST_HLS_PLAYLIST_TYPE_UNDEFINED,
  GST_HLS_PLAYLIST_TYPE_EVENT,
  GST_HLS_PLAYLIST_TYPE_VOD,
} GstHLSPlaylistType;

/**
 * GstHLSMediaPlaylist:
 *
 * Official term in RFC : "Media Playlist". A List of Media Segments.
 *
 * It can be used by either a variant stream (GstHLSVariantStream) or an
 * alternate rendition (GstHLSMedia).
 *
 * Note: Was called `GstM3u8` in legacy elements
 */

struct _GstHLSMediaPlaylist
{
  gchar *uri;                   /* actually downloaded URI */
  gchar *base_uri;              /* URI to use as base for resolving relative URIs.
                                 * This will be different to uri in case of redirects */
  /* Base Tag */
  gint version;                 /* EXT-X-VERSION (default 1) */

  /* Media Playlist Tags */
  GstClockTime targetduration;  /* EXT-X-TARGETDURATION, default GST_CLOCK_TIME_NONE */
  gint64 media_sequence;	/* EXT-X-MEDIA-SEQUENCE, MSN of the first Media
				   Segment in the playlist. */
  gint64 discont_sequence;	/* EXT-X-DISCONTINUITY-SEQUENCE. Default : 0 */
  gboolean has_ext_x_dsn;	/* EXT-X-DISCONTINUITY-SEQUENCE present and specified */
  gboolean endlist;             /* EXT-X-ENDLIST present */
  GstHLSPlaylistType type;	/* EXT-X-PLAYLIST-TYPE. Default:
				   GST_HLS_PLAYLIST_TYE_UNDEFINED */
  gboolean i_frame;		/* EXT-X-I-FRAMES-ONLY present. */

  gboolean allowcache;		/* deprecated EXT-X-ALLOW-CACHE */

  /* Overview of contained media segments */
  gboolean ext_x_key_present;	/* a valid EXT-X-KEY is present on at least one
				   media segment */
  gboolean ext_x_pdt_present;   /* a valid EXT-X-PROGRAM-DATE-TIME is present on
				   at least one media segment */

  GPtrArray *segments;		/* Array of GstM3U8MediaSegment */

  /* Generated information */
  GstClockTime duration;	/* The estimated total duration of all segments
				   contained in this playlist */

  gboolean reloaded;		/* If TRUE, this indicates that this playlist
				 * was reloaded but had identical content */

  /*< private > */
  GMutex lock;

  /* Copy of the incoming data that created this media playlist.
   * See gst_hls_media_playlist_has_same_data()  */
  gchar   *last_data;

  gint ref_count;               /* ATOMIC */
};

/* gst_hls_media_playlist_new: Internal function : Do not use from demuxer code, only for unit
 *               testing purposes */
GstHLSMediaPlaylist * gst_hls_media_playlist_new (const gchar * uri,
						  const gchar * base_uri);

GstHLSMediaPlaylist * gst_hls_media_playlist_ref (GstHLSMediaPlaylist * m3u8);

void                  gst_hls_media_playlist_unref (GstHLSMediaPlaylist * m3u8);

/**
 * GstM3U8MediaSegment:
 *
 * Official term in RFC : "Media Segment"
 *
 * Note : Naming in legacy elements was GstM3U8MediaFile
 */
struct _GstM3U8MediaSegment
{
  gchar *title;
  GstClockTimeDiff stream_time;	/* Computed stream time */
  GstClockTime duration;
  gchar *uri;
  gint64 sequence;		/* the sequence number of this segment */
  gint64 discont_sequence;	/* The Discontinuity Sequence Number of this segment */
  gboolean discont;             /* this file marks a discontinuity */
  gchar *key;
  guint8 iv[16];
  gint64 offset, size;
  gint ref_count;               /* ATOMIC */
  GstM3U8InitFile *init_file;   /* Media Initialization (hold ref) */
  GDateTime *datetime;		/* EXT-X-PROGRAM-DATE-TIME */
};

struct _GstM3U8InitFile
{
  gchar *uri;
  gint64 offset, size;
  guint ref_count;      /* ATOMIC */
};

GstM3U8MediaSegment *
gst_m3u8_media_segment_ref   (GstM3U8MediaSegment * mfile);

void
gst_m3u8_media_segment_unref (GstM3U8MediaSegment * mfile);


gboolean
gst_hls_media_playlist_has_same_data (GstHLSMediaPlaylist * m3u8,
				      gchar   * playlist_data);

GstHLSMediaPlaylist *
gst_hls_media_playlist_parse (gchar        * data,
			      const gchar  * uri,
			      const gchar  * base_uri);

void
gst_hls_media_playlist_recalculate_stream_time (GstHLSMediaPlaylist *playlist,
						GstM3U8MediaSegment *anchor);

GstM3U8MediaSegment *
gst_hls_media_playlist_sync_to_segment      (GstHLSMediaPlaylist * m3u8,
					     GstM3U8MediaSegment * segment);

gboolean
gst_hls_media_playlist_has_next_fragment    (GstHLSMediaPlaylist * m3u8,
					     GstM3U8MediaSegment * current,
					     gboolean  forward);

GstM3U8MediaSegment *
gst_hls_media_playlist_advance_fragment     (GstHLSMediaPlaylist * m3u8,
					     GstM3U8MediaSegment * current,
					     gboolean  forward);

GstM3U8MediaSegment *
gst_hls_media_playlist_get_starting_segment (GstHLSMediaPlaylist *self);

GstClockTime
gst_hls_media_playlist_get_duration         (GstHLSMediaPlaylist * m3u8);

gchar *
gst_hls_media_playlist_get_uri              (GstHLSMediaPlaylist * m3u8);

gboolean
gst_hls_media_playlist_is_live              (GstHLSMediaPlaylist * m3u8);

gboolean
gst_hls_media_playlist_get_seek_range       (GstHLSMediaPlaylist * m3u8,
					     gint64  * start,
					     gint64  * stop);

GstM3U8MediaSegment *
gst_hls_media_playlist_seek                 (GstHLSMediaPlaylist *playlist,
					     gboolean forward,
					     GstSeekFlags flags,
					     GstClockTimeDiff ts);
void
gst_hls_media_playlist_dump                 (GstHLSMediaPlaylist* self);

typedef enum
{
  GST_HLS_RENDITION_STREAM_TYPE_INVALID = -1,
  GST_HLS_RENDITION_STREAM_TYPE_AUDIO,
  GST_HLS_RENDITION_STREAM_TYPE_VIDEO,
  GST_HLS_RENDITION_STREAM_TYPE_SUBTITLES,
  GST_HLS_RENDITION_STREAM_TYPE_CLOSED_CAPTIONS,
  GST_HLS_N_MEDIA_TYPES
} GstHLSRenditionStreamType;

/**
 * GstHLSRenditionStream:
 *
 * Official term in RFC : "Renditions are alternate versions of the content,
 *   such as audio produced in different languages or video recorded from
 *   different camera angles."
 *
 * Note: Was named GstHLSMedia in legacy elements
 */

struct _GstHLSRenditionStream {
  GstHLSRenditionStreamType mtype;
  gchar *group_id;
  gchar *name;
  gchar *lang;
  gchar *uri;
  GstCaps *caps;
  gboolean is_default;
  gboolean autoselect;
  gboolean forced;

  gint ref_count;               /* ATOMIC */
};

GstHLSRenditionStream *
gst_hls_rendition_stream_ref   (GstHLSRenditionStream * media);

void
gst_hls_rendition_stream_unref (GstHLSRenditionStream * media);

const gchar *
gst_hls_rendition_stream_type_get_name (GstHLSRenditionStreamType mtype);


/**
 * GstHLSVariantStream:
 *
 * Official term in RFC :
 * """
 * A Master Playlist provides a set of Variant Streams, each of which describes
 *   a different version of the same content.
 *
 * A Variant Stream includes a Media Playlist that specifies media encoded at a
 *  particular bit rate, in a particular format, and at a particular resolution
 *  for media containing video.
 * """
 */
struct _GstHLSVariantStream {
  gchar *name;         /* This will be the "name" of the playlist, the original
                        * relative/absolute uri in a variant playlist */
  gchar *uri;
  gchar *codecs;
  GstCaps *caps;
  GstStreamType codecs_stream_type;	/* As defined by codecs */
  gint bandwidth;			/* bits per second */
  gint program_id;
  gint width;
  gint height;
  gboolean iframe;

  gint refcount;       /* ATOMIC */

  /* alternative renditions (names) */
  gchar *media_groups[GST_HLS_N_MEDIA_TYPES];

  /* List of gchar* fallback uri */
  GList *fallback;
};

/* Notes: #define are to avoid symbol clashes with legacy hlsdemux */

#define gst_hls_variant_stream_ref hls_variant_stream_ref
GstHLSVariantStream * hls_variant_stream_ref (GstHLSVariantStream * stream);

#define gst_hls_variant_stream_unref hls_variant_stream_unref
void                  hls_variant_stream_unref (GstHLSVariantStream * stream);

/**
 * GstHLSMasterPlaylist:
 *
 * Official term in RFC : "A Playlist is either a Media Playlist or a Master
 * Playlist."
 *
 * This is the top-level object, constructed by a manifest provided by external
 * means.
 */
struct _GstHLSMasterPlaylist
{
  /* Available variant streams, sorted by bitrate (low -> high) */
  GList    *variants;		/* GstHLSVariantStream */
  GList    *iframe_variants;	/* GstHLSVariantStream */

  /* Default variant, first in the list (originally, before sorting) */
  GstHLSVariantStream *default_variant;

  /* Full list of Available Alternative Rendition (GstHLSRenditionStream) */
  GList    *renditions;

  /* EXT-X-VERSION. 0 if unspecified */
  gint      version;

  /* TRUE if this playlist is a simple media playlist (and not a master
   * playlist). Implies that there is only a single variant and no alternate
   * rendition groups */
  gboolean  is_simple;

  /* TRUE if all variants have codecs specified */
  gboolean have_codecs;

  /*< private > */
  gchar   *last_data;		/* Copy of the incoming data that created this master playlist */

  gint      refcount;                    /* ATOMIC */
};

/* Notes: #define are to avoid symbol clashes with legacy hlsdemux */

#define gst_hls_master_playlist_new_from_data hls_master_playlist_new_from_data
GstHLSMasterPlaylist * hls_master_playlist_new_from_data (gchar       * data,
							  const gchar * base_uri);

#define gst_hls_master_playlist_get_variant_for_bitrate hls_master_playlist_get_variant_for_bitrate
GstHLSVariantStream *  hls_master_playlist_get_variant_for_bitrate (GstHLSMasterPlaylist * playlist,
								    GstHLSVariantStream  * current_variant,
								    guint                  bitrate,
								    guint                  min_bitrate);

#define gst_hls_master_playlist_get_common_caps hls_master_playlist_get_common_caps
GstCaps *              hls_master_playlist_get_common_caps (GstHLSMasterPlaylist *playlist);

#define gst_hls_master_playlist_unref hls_master_playlist_unref
void                   hls_master_playlist_unref (GstHLSMasterPlaylist * playlist);


/* Time Mapping
 *
 * Used to map GStreamer times to internal segment timestamps
 */
struct _GstHLSTimeMap {
  /* DISCONT SEQUENCE NUMBER */
  gint64 dsn;

  /* The stream time (used for gst timestamps, gst segments, seeking ...) */
  GstClockTime stream_time;

  /* The optional Program Date Time reference */
  GDateTime *pdt;

  /* The internal time (ex: mpeg-ts PTS) */
  GstClockTime internal_time;
};

GstStreamType          gst_stream_type_from_hls_type (GstHLSRenditionStreamType stype);
GstStreamType          gst_hls_get_stream_type_from_structure (GstStructure *structure);
GstStreamType          gst_hls_get_stream_type_from_caps (GstCaps *caps);

#if !GLIB_CHECK_VERSION(2, 62, 0)
static inline gchar* g_date_time_format_iso8601(GDateTime* datetime) {
  GString* outstr = NULL;
  gchar* main_date = NULL;
  gint64 offset;

  // Main date and time.
  main_date = g_date_time_format(datetime, "%Y-%m-%dT%H:%M:%S");
  outstr = g_string_new(main_date);
  g_free(main_date);

  // Timezone. Format it as `%:::z` unless the offset is zero, in which case
  // we can simply use `Z`.
  offset = g_date_time_get_utc_offset(datetime);

  if (offset == 0) {
    g_string_append_c(outstr, 'Z');
  } else {
    gchar* time_zone = g_date_time_format(datetime, "%:::z");
    g_string_append(outstr, time_zone);
    g_free(time_zone);
  }

  return g_string_free(outstr, FALSE);
}
#endif


G_END_DECLS

#endif /* __M3U8_H__ */
