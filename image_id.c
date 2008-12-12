/*
 * Image ID - Calculate MusicBrainz disc TOC numbers from CD Images
 * Copyright (c) 2008 Calvin Walton
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 * 
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

/*
 * Compile with:
 * gcc image_disc_id.c `pkg-config --cflags --libs libdiscid libmirage`
 */

#include <mirage.h>
#include <discid/discid.h>

#include <stdbool.h>
#include <stdlib.h>

#define PACKAGE "Image ID"
#define VERSION "1.0"
#define COPYRIGHT "Copyright (c) 2008 Calvin Walton"

/* Print the version information for the program */
static void print_version();
/* Print the usage information for the program */
static void print_usage(char *progname);
/* Process information for a disc */
static bool process_disc(MIRAGE_Disc *disc, DiscId *discid);

int main(int argc, char *argv[]) {
	MIRAGE_Mirage *mirage;
	MIRAGE_DebugContext *mirage_debug = NULL;

	DiscId *discid;

	GObject *disc_gobj;
	MIRAGE_Disc *disc;

	gchar **file_list;
	int i, file_count;

	GError *error = NULL;

	print_version();
	if (argc < 2) {
		print_usage(argv[0]);
		return 1;
	}

	/* Initialize libraries */
	g_type_init();
	mirage = g_object_new(MIRAGE_TYPE_MIRAGE, NULL);
	discid = discid_new();

	/*
	mirage_debug = g_object_new(MIRAGE_TYPE_DEBUG_CONTEXT, NULL);
	mirage_debug_context_set_debug_mask(mirage_debug,
			MIRAGE_DEBUG_WARNING,
			NULL);
			*/

	file_count = argc - 1;
	file_list = malloc((file_count + 1) * sizeof (gchar *));
	for (i = 0; i < file_count; i++) {
		file_list[i] = argv[i+1];
	}
	file_list[file_count] = NULL;

	mirage_mirage_create_disc(mirage, file_list, &disc_gobj,
			G_OBJECT(mirage_debug), &error);
	if (error) {
		fprintf(stderr, "Cannot open image: %s\n", error->message);
		return 1;
	}

	disc = MIRAGE_DISC(disc_gobj);

	process_disc(disc, discid);

	return 0;
}

static void print_version() {
	fprintf(stderr, "%s %s\n%s\n", PACKAGE, VERSION, COPYRIGHT);
}

static void print_usage(char *progname) {
	fprintf(stderr,	"Usage:\n"
			"\t%s <cd-image> ...\n",
			progname);
}

static bool process_disc(MIRAGE_Disc *disc, DiscId *discid) {
	int sessions;
	GError *error = NULL;

	int first, last;
	int offsets[100] = {0};

	mirage_disc_get_number_of_sessions(disc, &sessions, &error);
	if (error) {
		fprintf(stderr, "Cannot get session count: %s\n", error->message);
		return false;
	}
	fprintf(stderr, "Disc contains %d sessions\n", sessions);

	for (int i = 0; i < sessions; i++) {
		GObject *session_gobj;
		MIRAGE_Session *session;

		int leadout_length, tracks, type;
		int session_number, first_track, start_sector, length;
		int offset;

		mirage_disc_get_session_by_index(disc, i, &session_gobj, &error);
		if (error) {
			fprintf(stderr, "Cannot get session %d: %s\n", i,
					error->message);
			continue;
		}
		session = MIRAGE_SESSION(session_gobj);

		mirage_session_layout_get_session_number(session, &session_number, NULL);
		mirage_session_layout_get_first_track(session, &first_track, NULL);
		mirage_session_layout_get_start_sector(session, &start_sector, NULL);
		mirage_session_layout_get_length(session, &length, NULL);

		fprintf(stderr, "session %d: layout: number %d, first track %d, start sector %d, length %d\n",
				i, session_number, first_track, start_sector, length);

		mirage_session_get_session_type(session, &type, &error);
		mirage_session_get_leadout_length(session, &leadout_length, NULL);
		mirage_session_get_number_of_tracks(session, &tracks, NULL);

		fprintf(stderr, "session %d: %d tracks, type %d, leadout length %d\n",
				i, tracks, type, leadout_length);

		offset = 0 - start_sector;
		fprintf(stderr, "session %d: calculated offset %d\n", i, offset);

		offsets[0] = length;
		first = last = first_track;

		for (int j = 0; j < tracks; j++) {
			GObject *track_gobj;
			MIRAGE_Track *track;

			int track_start, indices;
			int track_num, track_start_sector, track_length;

			mirage_session_get_track_by_index(session, j, &track_gobj, &error);
			if (error) {
				fprintf(stderr, "Cannot get track %d: %s\n", j, error->message);
				continue;
			}
			track = MIRAGE_TRACK(track_gobj);

			mirage_track_layout_get_track_number(track, &track_num, NULL);
			mirage_track_layout_get_start_sector(track, &track_start_sector, NULL);
			mirage_track_layout_get_length(track, &track_length, NULL);

			fprintf(stderr, "session %d: track %d: layout: number %d, start sector %d, length %d\n",
					i, j, track_num, track_start_sector + offset, track_length);

			mirage_track_get_track_start(track, &track_start, NULL);
			mirage_track_get_number_of_indices(track, &indices, NULL);

			fprintf(stderr, "session %d: track %d: track start %d, %d indicies\n",
					i, j,
					track_start + track_start_sector + offset,
					indices);

			offsets[track_num] = track_start + track_start_sector + offset;
			if (track_num > last)
				last = track_num;

			g_object_unref(track);
		}

		g_object_unref(session);
	}

	fprintf(stderr, "Full TOC: %d %d %d", first, last, offsets[0]);
	for (int i = first; i <= last; i++) {
		fprintf(stderr, " %d", offsets[i]);
	}
	fprintf(stderr, "\n");

	discid_put(discid, first, last, offsets);
	printf("%s\n", discid_get_id(discid));

	fprintf(stderr, "FreeDB: %s\n", discid_get_freedb_id(discid));
	fprintf(stderr, "%s\n", discid_get_submission_url(discid));

	return true;
}