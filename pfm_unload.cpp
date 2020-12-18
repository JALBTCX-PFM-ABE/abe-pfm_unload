
/*********************************************************************************************

    This is public domain software that was developed by or for the U.S. Naval Oceanographic
    Office and/or the U.S. Army Corps of Engineers.

    This is a work of the U.S. Government. In accordance with 17 USC 105, copyright protection
    is not available for any work of the U.S. Government.

    Neither the United States Government, nor any employees of the United States Government,
    nor the author, makes any warranty, express or implied, without even the implied warranty
    of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, or assumes any liability or
    responsibility for the accuracy, completeness, or usefulness of any information,
    apparatus, product, or process disclosed, or represents that its use would not infringe
    privately-owned rights. Reference herein to any specific commercial products, process,
    or service by trade name, trademark, manufacturer, or otherwise, does not necessarily
    constitute or imply its endorsement, recommendation, or favoring by the United States
    Government. The views and opinions of authors expressed herein do not necessarily state
    or reflect those of the United States Government, and shall not be used for advertising
    or product endorsement purposes.

*********************************************************************************************/


#include "pfm_unload.hpp"
#include "version.hpp"


typedef struct
{
  int32_t     file;
  int32_t     rec;
  int32_t     sub;
  uint32_t    val;
  uint8_t     cls;
} SORT_REC;


//  This is the file/rec sort function for sort.

static int32_t compare_file_and_rec_numbers (const SORT_REC &a, const SORT_REC &b)
{
    //  If the files aren't equal we sort on file.

    if (a.file != b.file) return (a.file < b.file ? 0 : 1);


    //  Otherwise we sort on the record number.

    return (a.rec < b.rec ? 0 : 1);
}



/***************************************************************************\
*                                                                           *
*   Module Name:        pfm_unload                                          *
*                                                                           *
*   Programmer(s):      Jan C. Depner                                       *
*                                                                           *
*   Date Written:       October 10, 1999                                    *
*                                                                           *
*   Purpose:            Unloads changes from a PFM to the original input    *
*                       files.                                              *
*                                                                           *
\***************************************************************************/

int32_t main (int32_t argc, char **argv)
{
  new pfm_unload (argc, argv);
}


void pfm_unload::usage ()
{
  QString msg = tr ("\nUsage: pfm_unload <PFM_HANDLE_FILE or PFM_LIST_FILE> [-u]\n"
                    "\nWhere:\n\n"
                    "\t-u  =  unload ALL data that has EVER been modified (warning - VERY, VERY slow)\n\n\n"
                    "\t************************** IMPORTANT WARNING ***********************************\n\n"
                    "\tNEVER, NEVER, NEVER use the -u option unless you have lost your input files and\n"
                    "\thad to replace them!\n"
                    "\tIf you think that you invalidated something in a PFM, unloaded it, then built a new\n"
                    "\tPFM, and it showed back up you've made a mistake.  The most common cause for this\n"
                    "\tapparent problem is that you didn't load everything the first time you loaded the PFM.\n"
                    "\tThe second most common cause is that you looked at the data in the editor and, just\n"
                    "\tby comparing pictures, decided that you had invalidated that point and it didn't\n"
                    "\tunload properly.  Once again, you have made a mistake!  The -u option WILL NOT fix\n"
                    "\tit.  The ONLY use for the -u option is in the event that you have lost your input\n"
                    "\tfiles after unloading the PFM.  In that case, pfm_unload will have reset the\n"
                    "\tPFM_MODIFIED flag in the bin record.  The -u option forces pfm_unload to read EVERY\n"
                    "\tpoint in the ndx file to find the PFM_MODIFIED depth records.\n"
                    "\tEven though ill informed pundits (you know who you are) will tell you that -u is the\n"
                    "\tonly way to insure that everything gets unloaded, they are wrong!\n\n"
                    "\t************************** IMPORTANT WARNING ***********************************\n\n");

  qWarning () << qPrintable (msg);


  //  The following are still active but undocumented.

  //-a  =  area of file to dump, N,S,E,W (normally only used from pfmView)
  //       Ex. -a 29.0,28.0,-80.0,-79.0
  //-s  =  start file, requires argument (ex. -s 10)
  //-Q  =  flag to indicate that this was called from another ABE program
}


pfm_unload::pfm_unload (int32_t argc, char **argv)
{
  int32_t                 i, j, k, status = 0, percent = 0, gsf_handle, old_percent = -1, recnum, total_bins, ret, rec,
                          in_count = 0, year, mon, mday, hour, min, sec, filter = 0, manual = 0, width, height,
                          selected = 0;
  int16_t                 type;
  PFM_OPEN_ARGS           open_args;
  char                    file[512], string[512], comment[512], c, cday[10], cmon[10];
  NV_I32_COORD2           ll, ur, coord;
  NV_F64_COORD2           xy;
  NV_F64_XYMBR            mbr;              
  BIN_RECORD              bin_record;
  DEPTH_RECORD            *depth;
  int32_t                 pfm_handle, start_file = 0, prev_file;
  uint8_t                 gsf = NVFalse, czmil = NVFalse, laz = NVFalse, error_on_update = NVFalse, dump_all = NVFalse, partial = NVFalse,
                          old_lidar = NVFalse, Qt = NVFalse;
  std::vector<SORT_REC>   sort_rec;
  int32_t                 sort_count = 0;
  extern char             *optarg;
  extern int              optind;
  char                    month[12][4] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};


  int32_t unload_gsf_file (int32_t pfm_handle, int16_t file_number, int32_t ping_number, int16_t beam_number, uint32_t validity, char *file);
  int32_t unload_hof_file (int16_t file_number, int32_t ping_number, int16_t beam_number, uint32_t validity, uint8_t cls, char *file, int16_t type);
  int32_t unload_tof_file (int16_t file_number, int32_t ping_number, int16_t beam_number, uint32_t validity, uint8_t cls, char *file);
  int32_t unload_czmil_file (int32_t pfm_handle, int16_t file_number, int32_t ping_number, int16_t beam_number, uint32_t validity, uint8_t cls, char *file);
  int32_t unload_las_file (int16_t file_number, int32_t ping_number, uint32_t validity, uint8_t cls, char *file, uint8_t *laz);
  int32_t unload_llz_file (int16_t file_number, int32_t ping_number, uint32_t validity, char *file);
  int32_t unload_dted_file (int16_t file_number, int32_t ping_number, int16_t beam_number, uint32_t validity, char *file);

  void check_gsf_file (char *);
  void check_czmil_file (char *);
  uint8_t check_las_file (char *);
  void check_hof_file (char *);
  void check_tof_file (char *);
  uint8_t check_llz_file (char *);
  uint8_t check_dted_file (char *);
  void close_last_file ();
  int32_t write_history (int32_t, char **, char *, char *, int32_t);



  printf ("\n\n %s \n\n\n", VERSION);



  while ((c = getopt (argc, argv, "Qus:a:")) != EOF)
    {
      switch (c)
        {
	case 'Q':
	  Qt = NVTrue;
	  break;

	case 'u':
	  dump_all = NVTrue;
	  break;

	case 's':
	  sscanf (optarg, "%d", &start_file);
	  break;

	case 'a':
	  sscanf (optarg, "%lf,%lf,%lf,%lf", &mbr.max_y, &mbr.min_y, &mbr.max_x, &mbr.min_x);
	  partial = NVTrue;
	  break;

	default:
	  usage ();
	  exit (-1);
	  break;
        }
    }


  //  Make sure we got the mandatory file name argument.

  if (optind >= argc)
    {
      usage ();
      exit (-1);
    }


  strcpy (open_args.list_path, argv[optind]);


  strcpy (out_str, tr ("\n\nPFM File : %1\n\n\n").arg (open_args.list_path).toUtf8 ());
  fprintf (stdout, "%s", out_str);
  fflush (stdout);


  //  Open the PFM files.

  open_args.checkpoint = 0;
  if ((pfm_handle = open_existing_pfm_file (&open_args)) < 0) pfm_error_exit (pfm_error);


  //  We have to check for the CZMIL and/or LAS classification attribute in case some of the modifications are to the classification
  //  instead of just the validity.

  int32_t lidar_class_attr = -1;
  for (int32_t i = 0 ; i < open_args.head.num_ndx_attr ; i++)
    {
      if (QString (open_args.head.ndx_attr_name[i]).contains ("CZMIL Classification") ||
          QString (open_args.head.ndx_attr_name[i]).contains ("LAS Classification") ||
          QString (open_args.head.ndx_attr_name[i]).contains ("HOF Classification status") ||
          QString (open_args.head.ndx_attr_name[i]).contains ("TOF Classification status"))
        lidar_class_attr = i;
    }


  //  Check for old lidar data - record number starts at 0 instead of 1.   This is to fix a screwup between IVS, Optech, and us.  For some unknown reason
  //  IVS chose to start numbering at 1 instead of 0.  Go figure.

  sscanf (open_args.head.date, "%s %s %d %d:%d:%d %d", cday, cmon, &mday, &hour, &min, &sec, &year);

  mon = 0;
  for (i = 0 ; i < 12 ; i++)
    {
      if (!strcmp (cmon, month[i])) 
	{
	  mon = i;
	  break;
	}
    }

  if (year < 2004 || (year == 2004 && (mon < 9 || (mon == 9 && mday < 30)))) old_lidar = NVTrue;


  //  Pre-certify all of the files for writing.

  k = get_next_list_file_number (pfm_handle);

  for (j = 0 ; j < k ; j++)
    {
      strcpy (out_str, tr ("Checking input file %1 of %2\r").arg (j + 1, 4, 10).arg (k, 4, 10).toUtf8 ());
      fprintf (stderr, "%s", out_str);
      fflush (stderr);


      if (read_list_file (pfm_handle, j, file, &type)) break;


      //  Bypass files marked as PFM_DELETED.

      if (file[0] != '*')
        {
          switch (type)
            {
            case PFM_GSF_DATA:
              check_gsf_file (file);
              break;

            case PFM_CZMIL_DATA:
              check_czmil_file (file);
              break;

            case PFM_LAS_DATA:
              check_las_file (file);
              break;

            case PFM_SHOALS_1K_DATA:
            case PFM_CHARTS_HOF_DATA:
              check_hof_file (file);
              break;

            case PFM_SHOALS_TOF_DATA:
              check_tof_file (file);
              break;

            case PFM_NAVO_LLZ_DATA:
              check_llz_file (file);
              break;

            case PFM_DTED_DATA:
              check_dted_file (file);
              break;
            }
        }
    }
  fprintf (stderr, "                                           \r");
  fflush (stderr);

  if (partial)
    {
      xy.y = mbr.min_y;
      xy.x = mbr.min_x;

      compute_index_ptr (xy, &ll, &open_args.head);

      if (ll.x < 0) ll.x = 0;
      if (ll.y < 0) ll.y = 0;


      xy.y = mbr.max_y;
      xy.x = mbr.max_x;
        
      compute_index_ptr (xy, &ur, &open_args.head);

      if (ur.x > open_args.head.bin_width) ur.x = open_args.head.bin_width;
      if (ur.y > open_args.head.bin_height) ur.y = open_args.head.bin_height;
    }
  else
    {
      ll.x = 0;
      ll.y = 0;
      ur.x = open_args.head.bin_width;
      ur.y = open_args.head.bin_height;
    }


  width = ur.x - ll.x;
  height = ur.y - ll.y;
  total_bins = width * height;


  strcpy (out_str, tr ("Reading PFM bin and index files.\n\n").toUtf8 ());
  fprintf (stderr, "%s", out_str);
  fflush (stderr);


  //  First we're going to sort all the modified records by file and record so that we don't thrash the disk when we unload.
  //  Please note that we don't unset the PFM_MODIFIED flag in the depth records for two reasons.  One, because if something screws up we can
  //  always use the brute force dump_all method even if someone has reset the bin PFM_MODIFIED flag.  Two, it is nice to keep track of just how
  //  many records have actually been modified.  The drawback to this is that if you modify a record in a bin that has been previously unloaded you
  //  will unload all of the PFM_MODIFIED records in the bin again.  I can live with this.  IVS and SAIC may do it differently.

  for (i = ll.y ; i < ur.y ; i++)
    {
      coord.y = i;

      for (j = ll.x ; j < ur.x ; j++)
        {
	  coord.x = j;

	  if (read_bin_record_index (pfm_handle, coord, &bin_record))
            {
	      fprintf (stderr,"%d %d %d %d\n",open_args.head.bin_width, open_args.head.bin_height, i, j);
	      fflush (stderr);
            }
	
          if (dump_all || (bin_record.validity & PFM_MODIFIED))
            {
	      if (!read_depth_array_index (pfm_handle, coord, &depth, &recnum))
                {
                  for (k = 0 ; k < recnum ; k++)
                    {
		      //  Don't unload any data marked as PFM_DELETED.

		      if (!(depth[k].validity & PFM_DELETED))
                        {
			  //  Unload data marked in the depth record as PFM_MODIFIED.

			  if (depth[k].validity & PFM_MODIFIED)
                            {
			      //  Only unload those files whose file numbers are greater than start_file.

			      if (!start_file || (depth[k].file_number > start_file))
                                {
				  read_list_file (pfm_handle, depth[k].file_number, file, &type);

				  if (file[0] != '*')
                                    {
                                      if (depth[k].validity & PFM_FILTER_INVAL) filter++;
                                      if (depth[k].validity & PFM_MANUALLY_INVAL) manual++;
                                      if (depth[k].validity & PFM_SELECTED_SOUNDING) selected++;

                                      try
                                        {
                                          sort_rec.resize (sort_count + 1);
                                        }
                                      catch (std::bad_alloc&)
                                        {
                                          perror ("Allocating sort_rec in main.c");
                                          exit (-1);
                                        }

                                      sort_rec[sort_count].file = depth[k].file_number;
                                      sort_rec[sort_count].rec = depth[k].ping_number;
                                      sort_rec[sort_count].sub = depth[k].beam_number;
                                      sort_rec[sort_count].val = depth[k].validity;
                                      sort_rec[sort_count].cls = 0;


                                      //  Check for classification change.

                                      if (lidar_class_attr >= 0) sort_rec[sort_count].cls = (uint8_t) NINT (depth[k].attr[lidar_class_attr]);


                                      sort_count++;
                                    }
                                }
                            }
                        }
                    }
		  free (depth);
                }
            }

	  percent = ((float) ((i - ll.y) * width + (j - ll.x)) / (float) total_bins) * 100.0;
	  if (old_percent != percent)
            {
	      if (Qt)
		{
                  //  WARNING: Never translate the Qt strings.  They are used for IPC.

		  fprintf (stderr, "%d%%\r", percent);
		}
	      else
		{
                  strcpy (out_str, tr ("%1%% read    \r").arg (percent, 3, 10, QChar ('0')).toUtf8 ());
                  fprintf (stderr, "%s", out_str);
		}
	      fflush (stderr);
	      old_percent = percent;
            }
        }
    }


  //  Sort the records so we can write to each file in order.

  sort (sort_rec.begin (), sort_rec.end (), compare_file_and_rec_numbers);


  if (Qt)
    {
      //  WARNING: Never translate the Qt strings.  They are used for IPC.

      fprintf (stderr, "100%%\r");
    }
  else
    {
      strcpy (out_str, tr ("100%% read         \n").toUtf8 ());
      fprintf (stderr, "%s", out_str);
    }


  strcpy (out_str, tr ("Updating input files.\n\n").toUtf8 ());
  fprintf (stderr, "%s", out_str);
  fflush (stderr);


  prev_file = -999;

  for (i = 0 ; i < sort_count ; i++)
    {
      if (sort_rec[i].file != prev_file)
        {
          read_list_file (pfm_handle, sort_rec[i].file, file, &type);

          prev_file = sort_rec[i].file;
        }


      switch (type)
        {
        case PFM_GSF_DATA:
          status = unload_gsf_file (pfm_handle, sort_rec[i].file, sort_rec[i].rec, sort_rec[i].sub, sort_rec[i].val, file);
          if (!status) gsf = NVTrue;
          break;

        case PFM_SHOALS_1K_DATA:
        case PFM_CHARTS_HOF_DATA:

          //  New lidar loads/unloads done starting at record 1.

          if (old_lidar)
            {
              rec = sort_rec[i].rec + 1;
            }
          else
            {
              rec = sort_rec[i].rec;
            }

          status = unload_hof_file (sort_rec[i].file, rec, sort_rec[i].sub, sort_rec[i].val, sort_rec[i].cls, file, type);
          break;

        case PFM_SHOALS_TOF_DATA:

          //  New lidar loads/unloads done starting at record 1.

          if (old_lidar)
            {
              rec = sort_rec[i].rec + 1;
            }
          else
            {
              rec = sort_rec[i].rec;
            }

          status = unload_tof_file (sort_rec[i].file, rec, sort_rec[i].sub, sort_rec[i].val, sort_rec[i].cls, file);
          break;

        case PFM_CZMIL_DATA:
          status = unload_czmil_file (pfm_handle, sort_rec[i].file, sort_rec[i].rec, sort_rec[i].sub, sort_rec[i].val, sort_rec[i].cls, file);
          if (!status) czmil = NVTrue;
          break;

        case PFM_LAS_DATA:
          status = unload_las_file (sort_rec[i].file, sort_rec[i].rec, sort_rec[i].val, sort_rec[i].cls, file, &laz);
          break;

        case PFM_NAVO_LLZ_DATA:
          status = unload_llz_file (sort_rec[i].file, sort_rec[i].rec, sort_rec[i].val, file);
          break;

        case PFM_DTED_DATA:
          status = unload_dted_file (sort_rec[i].file, sort_rec[i].rec, sort_rec[i].sub, sort_rec[i].val, file);
          break;
        }

      if (status) error_on_update = NVTrue;

      in_count++;


      percent = ((float) i / (float) sort_count) * 100.0;
      if (old_percent != percent)
        {
          if (Qt)
            {
              //  WARNING: Never translate the Qt strings.  They are used for IPC.

              fprintf (stderr, "%d%%\r", percent);
            }
          else
            {
              strcpy (out_str, tr ("%1%% written    \r").arg (percent, 3, 10, QChar ('0')).toUtf8 ());
              fprintf (stderr, "%s", out_str);
            }
          fflush (stderr);
          old_percent = percent;
        }
    }


  sort_rec.clear ();


  close_last_file ();


  if (Qt)
    {
      //  WARNING: Never translate the Qt strings.  They are used for IPC.

      fprintf (stderr, "100%%\r");
    }
  else
    {
      strcpy (out_str, tr ("100%% written         \n").toUtf8 ());
      fprintf (stderr, "%s", out_str);
    }

  strcpy (out_str, tr ("\n\n%1 edited points read\n"
                       "%2 filter edited\n"
                       "%3 manually edited\n"
                       "%4 selected soundings\n\n\n").arg (in_count).arg (filter).arg (manual).arg (selected).toUtf8 ());
  fprintf (stderr, "%s", out_str);
  fflush (stderr);


  //  If we had any GSF, CZMIL, or LAZ files, flush the last one and close it.

  if (gsf)
    {
      if (unload_gsf_file (pfm_handle, -1, -1, -1, 0, NULL) == -1) error_on_update = NVTrue;


      k = get_next_list_file_number (pfm_handle);


      //  Write history records to GSF files.

      if (!partial)
        {
          strcpy (out_str, tr ("Writing history records to GSF files\n\n").toUtf8 ());
          fprintf (stderr, "%s", out_str);
	  fflush (stderr);

	  for (j = 0 ; j < k ; j++)
            {
	      if (read_list_file (pfm_handle, j, file, &type)) break;


	      if (type == PFM_GSF_DATA)
                {
		  fprintf (stderr, "%s\n", file);
		  fflush (stderr);


		  //  Open the file non-indexed so that we can write a history record.

		  if (gsfOpen (file, GSF_UPDATE, &gsf_handle))
                    {
		      gsfPrintError (stderr);
		      fflush (stderr);
		      gsfClose (gsf_handle);
                    }
		  else
                    {
		      //  Write a history record describing the polygon and bin size.

		      sprintf (comment, "PFM edited:\nBin size - %fm\nPolygon points - %d\n", open_args.head.bin_size_xy, open_args.head.polygon_count);

		      for (i = 0 ; i < open_args.head.polygon_count ; i++)
                        {
			  sprintf (string, "Point %03d - %f , %f\n", i, open_args.head.polygon[i].y, open_args.head.polygon[i].x);
			  strcat (comment, string);
                        }

		      ret = write_history (argc, argv, comment, file, gsf_handle);
		      if (ret)
                        {
                          strcpy (out_str, tr ("Error: %1 - writing gsf history record\n").arg (ret).toUtf8 ());
                          fprintf (stderr, "%s", out_str);
			  fflush (stderr);
                        }

		      gsfClose (gsf_handle);
                    }
                }
            }
        }
    }
  else if (czmil)
    {
      if (unload_czmil_file (pfm_handle, -1, -1, -1, 0, -1, NULL)) error_on_update = NVTrue;
    }
  else if (laz)
    {
      if (unload_las_file (-1, -1, 0, 0, NULL, &laz)) error_on_update = NVTrue;
    }


  if (error_on_update)
    {
      strcpy (out_str, tr ("\n\nError unloading one or more points!\n").toUtf8 ());
      fprintf (stderr, "%s", out_str);

      if (!Qt)
	{
          strcpy (out_str, tr ("Do you want to reset modified flags anyway [y/n] ? ").toUtf8 ());
          fprintf (stderr, "%s", out_str);
          fflush (stderr);

	  ngets (string, sizeof (string), stdin);
	  fprintf(stderr, "\n\n");
	  if (string[0] == 'y' || string[0] == 'Y') error_on_update = NVFalse;
	}

      fflush (stderr);
    }



  if (!error_on_update)
    {
      strcpy (out_str, tr ("Resetting modified flags in PFM bin file.\n\n").toUtf8 ());
      fprintf (stderr, "%s", out_str);
      fflush (stderr);


      //  Go back through the bin file and unset the modified flag.  This is not done as it is read in because the write to the input file 
      //  might fail.

      for (i = ll.y ; i < ur.y ; i++)
        {
	  coord.y = i;

	  for (j = ll.x ; j < ur.x ; j++)
            {
	      coord.x = j;

	      if ((status = read_bin_record_index (pfm_handle, coord, &bin_record))) break;

	      if (bin_record.validity & PFM_MODIFIED)
                {
		  bin_record.validity &= ~PFM_MODIFIED;
		  write_bin_record_index (pfm_handle, &bin_record);
                }

	      percent = ((float) ((i - ll.y) * width + (j - ll.x)) / (float) total_bins) * 100.0;
	      if (old_percent != percent)
		{
		  if (Qt)
		    {
                      //  WARNING: Never translate the Qt strings.  They are used for IPC.

		      fprintf (stderr, "%d%%\r", percent);
		    }
		  else
		    {
                      strcpy (out_str, tr ("%1%% processed    \r").arg (percent, 3, 10, QChar ('0')).toUtf8 ());
                      fprintf (stderr, "%s", out_str);
		    }

		  fflush (stderr);
		  old_percent = percent;
		}
            }
        }

      if (Qt)
	{
          //  WARNING: Never translate the Qt strings.  They are used for IPC.

	  fprintf (stderr,"100%%\r");
	}
      else
	{
          strcpy (out_str, tr ("100%% processed         \n\n\n").toUtf8 ());
          fprintf (stderr, "%s", out_str);
	}

      fflush (stderr);
    }


  //  Close the PFM files.

  close_pfm_file (pfm_handle);
}


pfm_unload::~pfm_unload ()
{
}
