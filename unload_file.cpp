
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>

#include <QtCore>

#include "FileHydroOutput.h"
#include "FileTopoOutput.h"

#include "nvutility.h"

#include "pfm.h"

#include "gsf.h"

#include "czmil.h"

#include <lasreader.hpp>
#include <slas.hpp>

#include "llz.h"

#include "dted.h"


static int32_t     last_handle = -1;
static FILE        *last_fp = NULL;
static int32_t     last_type = -1;
static uint8_t     lz_available = NVFalse;
static QString     lzName;
static char        out_str[2048];


void close_last_file ()
{
  switch (last_type)
    {

      //  Don't close GSF, CZMIL or LAS/LAZ here.  We need to flush the last buffer (in pfm_unload.cpp).

    case PFM_GSF_DATA:
    case PFM_CZMIL_DATA:
    case PFM_LAS_DATA:
      break;

    case PFM_SHOALS_1K_DATA:
    case PFM_CHARTS_HOF_DATA:
    case PFM_SHOALS_TOF_DATA:
    case PFM_DTED_DATA:
      fclose (last_fp);
      break;

    case PFM_NAVO_LLZ_DATA:
      close_llz (last_handle);
      break;
    }
}


void check_gsf_file (char *path)
{
  int32_t           stat, gsf_handle;
  char              ndx_file[256];


  //  Remove the GSF index file.  We have been having problems with corrupted GSF index files (cause unknown) so we want to get rid of
  //  it and let the GSF library gen us up a new one.

  strcpy (ndx_file, path);
  ndx_file[strlen (ndx_file) - 3] = 'n';
  stat = remove (ndx_file);

  if (stat && errno != ENOENT)
    {
      strcpy (out_str, QObject::tr ("\n\nUnable to remove the GSF index file.\n").toUtf8 ());
      fprintf (stderr, "%s", out_str);
      perror (ndx_file);
      exit (-1);
    }


  //  Open the GSF file indexed so the library will generate another index file.

  if (gsfOpen (path, GSF_UPDATE_INDEX, &gsf_handle)) 
    {
      strcpy (out_str, QObject::tr ("\n\nUnable to open file %1\n").arg (path).toUtf8 ());
      fprintf (stderr, "%s", out_str);
      gsfPrintError (stderr);
      exit (-1);
    }

  gsfClose (gsf_handle);
}



int32_t unload_gsf_file (int32_t pfm_handle, int16_t file_number, int32_t ping_number, int16_t beam_number, uint32_t validity, char *file)
{
  static gsfDataID        gsf_data_id;
  static gsfRecords       gsf_records;
  static int32_t          gsf_handle, prev_ping_number = -1, numrecs = 0, numfiles = 0;
  static int16_t          prev_file_number = -1;
  static char             prev_file[512];


  void pfm_to_gsf_flags (uint32_t pfm_flags, uint8_t *gsf_flags);


  //  If we've changed files, write out the last record, close the old file, and open the new one.

  if (file_number != prev_file_number)
    {
      numfiles = get_next_list_file_number (pfm_handle);


      //  Write last record and close old file.

      if (prev_file_number != -1)
        {
          if (gsfWrite (gsf_handle, &gsf_data_id, &gsf_records) == -1)
            {
              strcpy (out_str, QObject::tr ("\nFile : %1\n").arg (prev_file).toUtf8 ());
              fprintf (stderr, "%s", out_str);
              gsfPrintError (stderr);
              strcpy (out_str, QObject::tr ("Record number = %L1\n\n").arg (gsf_data_id.record_number).toUtf8 ());
              fprintf (stderr, "%s", out_str);
              gsfClose (gsf_handle);
              fflush (stderr);
              return (-1);
            }

          gsfClose (gsf_handle);


          //  Bail out if no more files.

          if (file_number < 0 && file == NULL) return (0);


          prev_ping_number = -1;
        }


      //  If we have no more files, bail out.

      if (file_number < 0 || file_number > numfiles)
        {
          strcpy (out_str, QObject::tr ("\nFile number %1 out of range: 0 - %2\n").arg (file_number).arg (numfiles).toUtf8 ());
          fprintf (stderr, "%s", out_str);
          fflush (stderr);
          return (-1);
        }


      prev_file_number = file_number;
      strcpy (prev_file, file);


      //  Open the next file.

      if (gsfOpen (file, GSF_UPDATE_INDEX, &gsf_handle))
        {
          strcpy (out_str, QObject::tr ("\nFile : %1\n").arg (file).toUtf8 ());
          fprintf (stderr, "%s", out_str);
          gsfPrintError (stderr);
          if (gsf_handle) gsfClose (gsf_handle);
          fflush (stderr);
          return (-1);
        }

      numrecs = gsfGetNumberRecords (gsf_handle, GSF_RECORD_SWATH_BATHYMETRY_PING);


      last_handle = gsf_handle;
      last_type = PFM_GSF_DATA;
    }


  //  If we changed pings, write the old ping and read the new one.

  if (ping_number != prev_ping_number)
    {
      //  Write the old ping.

      if (prev_ping_number != -1)
        {
          if (gsfWrite (gsf_handle, &gsf_data_id, &gsf_records) == -1)
            {
              strcpy (out_str, QObject::tr ("\nFile : %1\n").arg (prev_file).toUtf8 ());
              fprintf (stderr, "%s", out_str);
              gsfPrintError (stderr);
              strcpy (out_str, QObject::tr ("Record number = %L1\n\n").arg (gsf_data_id.record_number).toUtf8 ());
              fprintf (stderr, "%s", out_str);
              prev_ping_number = -1;
              fflush (stderr);
              return (-1);
            }
        }


      if (ping_number < 0 || ping_number > numrecs) 
        {
          strcpy (out_str, QObject::tr ("\nFile : %1\nPing %L2 out of range: 0 - %L3\n\n").arg (file).arg (ping_number).arg (numrecs).toUtf8 ());
          fprintf (stderr, "%s", out_str);
          fflush (stderr);
          return (-1);
        }


      //  Read the new one.

      gsf_data_id.recordID = GSF_RECORD_SWATH_BATHYMETRY_PING;
      gsf_data_id.record_number = ping_number;

      if (gsfRead (gsf_handle, GSF_RECORD_SWATH_BATHYMETRY_PING, &gsf_data_id, &gsf_records, NULL, 0) == -1)
        {
          strcpy (out_str, QObject::tr ("\nFile : %1\n").arg (file).toUtf8 ());
          fprintf (stderr, "%s", out_str);
          gsfPrintError (stderr);
          prev_ping_number = ping_number;
          fflush (stderr);
          return (-1);
        }

      prev_ping_number = ping_number;
    }


  if (beam_number < 1 || beam_number > gsf_records.mb_ping.number_beams)
    {
      strcpy (out_str, QObject::tr ("\nFile : %1\n\nPing : %L2\nBeam %L3 out of range: 0 - %L4\n\n").arg (file).arg
              (ping_number).arg (beam_number).arg (gsf_records.mb_ping.number_beams).toUtf8 ());
      fprintf (stderr, "%s", out_str);
      fflush (stderr);
      return (-1);
    }


  //  Set the beam flags.

  pfm_to_gsf_flags (validity, &gsf_records.mb_ping.beam_flags[beam_number - 1]);


  return (0);
}



void check_hof_file (char *path)
{
  FILE           *hof_fp;


  //  Open the CHARTS .hof file.

  if ((hof_fp = open_hof_file (path)) == NULL)
    {
      strcpy (out_str, QObject::tr ("\n\nUnable to open the CHARTS .hof file.\n").toUtf8 ());
      fprintf (stderr, "%s", out_str);
      perror (path);
      exit (-1);
    }

  fclose (hof_fp);
}



int32_t unload_hof_file (int16_t file_number, int32_t ping_number, int16_t beam_number, uint32_t validity, uint8_t cls, char *file, int16_t type)
{
  static int16_t          prev_file_number = -1;
  static FILE             *hof_fp;
  HOF_HEADER_T            head;
  HYDRO_OUTPUT_T          record;


  //  If we've changed files close the old file and open the new one.

  if (file_number != prev_file_number)
    {
      //  Close old file.

      if (prev_file_number != -1) fclose (hof_fp);


      //  Open the CHARTS file.

      if ((hof_fp = open_hof_file (file)) == NULL)
        {
          strcpy (out_str, QObject::tr ("\nError opening CHARTS .hof file.\n").toUtf8 ());
          fprintf (stderr, "%s", out_str);
          perror (file);
          prev_file_number = -1;
          fflush (stderr);
          return (-1);
        }


      last_fp = hof_fp;
      last_type = type;


      hof_read_header (hof_fp, &head);


      prev_file_number = file_number;
    }


  hof_read_record (hof_fp, ping_number, &record);


  //  If we used PFM_CHARTS_HOF_DATA we don't honor primary and secondary depths.  They are both valid unless
  //  marked otherwise.

  if (type == PFM_CHARTS_HOF_DATA)
    {
      if (validity & PFM_SELECTED_FEATURE) record.suspect_status |= SUSPECT_STATUS_FEATURE_BIT;


      //  If the PFM record is marked as PFM_SELECTED_SOUNDING this means that the LiDAR classification has been modified.

      if (validity & PFM_SELECTED_SOUNDING) record.classification_status = cls;


      if (beam_number)
        {
          if (validity & PFM_INVAL)
            {
              if (record.sec_abdc > 0) record.sec_abdc = -record.sec_abdc;


              //  Now we check to see if the user has invalidated a point that was originally invalidated by GCS (abdc < 70)
              //  but was manually overridden (+100).

              if (record.sec_abdc < -100) record.sec_abdc += 100;
            }
          else
            {
              if (record.sec_abdc < 0) record.sec_abdc = -record.sec_abdc;


              //  Now we check to see if the user has manually overridden a point marked invalid by GCS (abdc < 70).  As long as 
              //  the depth is valid and the abdc isn't 13 or 10, we add 100 to the value to force it to be valid.

              if (record.sec_abdc < 70 && record.correct_sec_depth > -998.0 && record.sec_abdc != 13 && record.sec_abdc != 10) record.sec_abdc += 100;


              //  We need to see if GCS has set the AU_STATUS_DELETED bit in the status field.  If it has and we want this record
              //  to be valid we have to override that bit.

              if (record.status & AU_STATUS_DELETED_BIT) record.status &= 0xfe;
            }
        }
      else
        {
          if (validity & PFM_INVAL)
            {
              if (record.abdc > 0) record.abdc = -record.abdc;


              //  Now we check to see if the user has invalidated a point that was originally invalidate by GCS (abdc < 70)
              //  but was manually overridden (+100).

              if (record.abdc < -100) record.abdc += 100;
            }
          else
            {
              if (record.abdc < 0) record.abdc = -record.abdc;


              //  Now we check to see if the user has manually overridden a point marked invalid by GCS (abdc < 70).  As long as 
              //  the depth is valid and the abdc isn't 13 or 10, we add 100 to the value to force it to be valid.

              if (record.abdc < 70 && record.correct_depth > -998.0 && record.abdc != 13 && record.abdc != 10) record.abdc += 100;


              //  We need to see if GCS has set the AU_STATUS_DELETED bit in the status field.  If it has and we want this record
              //  to be valid we have to override that bit.

              if (record.status & AU_STATUS_DELETED_BIT) record.status &= 0xfe;
            }
        }
    }
  else
    {
      if (validity & PFM_SELECTED_FEATURE) record.suspect_status |= SUSPECT_STATUS_FEATURE_BIT;


      //  If the PFM record is marked as PFM_SELECTED_SOUNDING this means that the LiDAR classification has been modified.

      if (validity & PFM_SELECTED_SOUNDING) record.classification_status = cls;


      //  Another thing with the hof data - if the deleted bit is set we also set the abdc to a negative (or vice-versa).

      if (validity & PFM_INVAL)
        {
          if (record.abdc > 0) record.abdc = -record.abdc;


          //  Now we check to see if the user has invalidated a point that was originally invalidate by GCS (abdc < 70)
          //  but was manually overridden (+100).

          if (record.abdc < -100) record.abdc += 100;
        }
      else
        {
          if (record.abdc < 0) record.abdc = -record.abdc;


          //  Now we check to see if the user has manually overridden a point marked invalid by GCS (abdc < 70).  As long as 
          //  the depth is valid and the abdc isn't 13 or 10, we add 100 to the value to force it to be valid.

          if (record.abdc < 70 && record.correct_depth > -998.0 && record.abdc != 13 && record.abdc != 10) record.abdc += 100;


          //  We need to see if GCS has set the AU_STATUS_DELETED bit in the status field.  If it has and we want this record
          //  to be valid we have to override that bit.

          if (record.status & AU_STATUS_DELETED_BIT) record.status &= 0xfe;
        }
    }


  if (!hof_write_record (hof_fp, ping_number, &record))
    {
      strcpy (out_str, QObject::tr ("\n\nUnable to write to the CHARTS .hof file.\n").toUtf8 ());
      fprintf (stderr, "%s", out_str);
      perror (file);
      exit (-1);
    }

  return (0);
}



void check_tof_file (char *path)
{
  FILE           *tof_fp;


  //  Open the CHARTS .tof file.

  if ((tof_fp = open_tof_file (path)) == NULL)
    {
      strcpy (out_str, QObject::tr ("\n\nUnable to open the CHARTS .tof file.\n").toUtf8 ());
      fprintf (stderr, "%s", out_str);
      perror (path);
      exit (-1);
    }

  fclose (tof_fp);
}



int32_t unload_tof_file (int16_t file_number, int32_t ping_number, int16_t beam_number, uint32_t validity, uint8_t cls, char *file)
{
  static int16_t          prev_file_number = -1;
  static FILE             *tof_fp;
  static int32_t          prev_ping = -1;
  TOF_HEADER_T            head;
  TOPO_OUTPUT_T           record;


  //  If we've changed files close the old file and open the new one.

  if (file_number != prev_file_number)
    {
      //  Close old file.

      if (prev_file_number != -1) fclose (tof_fp);


      //  Open the CHARTS file.

      if ((tof_fp = open_tof_file (file)) == NULL)
        {
          strcpy (out_str, QObject::tr ("\nError opening CHARTS .tof file.\n").toUtf8 ());
          fprintf (stderr, "%s", out_str);
          perror (file);
          prev_file_number = -1;
          fflush (stderr);
          return (-1);
        }


      last_fp = tof_fp;
      last_type = PFM_SHOALS_TOF_DATA;


      tof_read_header (tof_fp, &head);


      prev_file_number = file_number;
      prev_ping = -1;
    }


  if (ping_number != prev_ping) tof_read_record (tof_fp, ping_number, &record);
  prev_ping = ping_number;


  if (beam_number)
    {
      //  If the PFM record is marked as PFM_SELECTED_SOUNDING this means that the LiDAR classification has been modified.

      if (validity & PFM_SELECTED_SOUNDING) record.classification_status = cls;


      if (validity & PFM_INVAL)
        {
          if (record.conf_last > 0) record.conf_last = -record.conf_last;


          //  If the difference between the first and last return is less than 5 cm we didn't load the first return
          //  so we want to make sure that the first return gets set with the same validity as the last return if,
          //  and only if, the last return is set to invalid.  That is, we don't want to validate a first return
          //  that is within 5 cm of the last return if the last return is valid, but we do want to invalidate the
          //  first return if the last return is invalid and they are within 5 cm of each other.

          if (record.conf_last < 0 && fabs ((double) (record.elevation_last - record.elevation_first)) < 0.05)
            {
              if (record.conf_first > 0) record.conf_first = -record.conf_first;
            }
        }
      else
        {
          if (record.conf_last < 0) record.conf_last = -record.conf_last;
        }
    }
  else
    {
      //  If the PFM record is marked as PFM_SELECTED_SOUNDING this means that the LiDAR classification has been modified.

      if (validity & PFM_SELECTED_SOUNDING) record.classification_status = cls;


      if (validity & PFM_INVAL)
        {
          if (record.conf_first > 0) record.conf_first = -record.conf_first;
        }
      else
        {
          if (record.conf_first < 0) record.conf_first = -record.conf_first;
        }
    }


  if (!tof_write_record (tof_fp, ping_number, &record))
    {
      strcpy (out_str, QObject::tr ("\n\nUnable to write to the CHARTS .tof file.\n").toUtf8 ());
      fprintf (stderr, "%s", out_str);
      perror (file);
      exit (-1);
    }

  return (0);
}



void check_czmil_file (char *path)
{
  int32_t            czmil_hnd;
  CZMIL_CPF_Header   czmil_cpf_header;


  //  Open the CZMIL file.

  if ((czmil_hnd = czmil_open_cpf_file (path, &czmil_cpf_header, CZMIL_UPDATE)) < 0)
    {
      fprintf (stderr, "%s : %s\n", path, czmil_strerror ());
      return;
    }

  czmil_close_cpf_file (czmil_hnd);

  return;
}




int32_t unload_czmil_file (int32_t pfm_handle, int16_t file_number, int32_t ping_number, int16_t beam_number, uint32_t validity, uint8_t cls, char *file)
{
  static int16_t          prev_file_number = -1;
  static int32_t          cpf_handle, prev_ping_number = -1, numrecs = 0, numfiles = 0;
  static char             prev_file[512];
  static CZMIL_CPF_Header head;
  static CZMIL_CPF_Data   record;
  int32_t                 channel, retnum;


  //  If we've changed files, write out the last record, close the old file, and open the new one.

  if (file_number != prev_file_number)
    {
      numfiles = get_next_list_file_number (pfm_handle);


      //  Write last record and close old file.

      if (prev_file_number != -1)
        {
          if (czmil_update_cpf_return_status (cpf_handle, prev_ping_number, &record) < 0)
            {
              czmil_perror ();
              return (-1);
            }

          czmil_close_cpf_file (cpf_handle);


          //  If we have no more files, bail out (called with a file_number of -1 to force unloading of last record).

          if (file_number < 0 && file == NULL) return (0);


          prev_ping_number = -1;
        }


      //  Make sure that the file number is valid.

      if (file_number < 0 || file_number > numfiles)
        {
          strcpy (out_str, QObject::tr ("\nFile number %L1 out of range: 0 - %L2\n").arg (file_number).arg (numfiles).toUtf8 ());
          fprintf (stderr, "%s", out_str);
          fflush (stderr);
          return (-1);
        }


      //  Open the next file.

      if ((cpf_handle = czmil_open_cpf_file (file, &head, CZMIL_UPDATE)) < 0)
        {
          fprintf (stderr, "%s\n", czmil_strerror ());
          prev_file_number = -1;
          return (-1);
        }


      numrecs = head.number_of_records;


      prev_file_number = file_number;
      strcpy (prev_file, file);

      last_handle = cpf_handle;
      last_type = PFM_CZMIL_DATA;
    }


  //  If we changed records (or just opened a new file), write the old record and read the new one.

  if (ping_number != prev_ping_number)
    {
      //  Write the old ping.

      if (prev_ping_number != -1)
        {
          if (czmil_update_cpf_return_status (cpf_handle, prev_ping_number, &record) < 0)
            {
              czmil_perror ();
              return (-1);
            }
        }


      //  Make sure that the ping number is valid.

      if (ping_number < 0 || ping_number > numrecs) 
        {
          strcpy (out_str, QObject::tr ("\nFile : %1\nRecord %L2 out of range: 0 - %L3\n\n").arg (file).arg (ping_number).arg (numrecs).toUtf8 ());
          fprintf (stderr, "%s", out_str);
          fflush (stderr);
          return (-1);
        }


      //  Read the new one.

      if (czmil_read_cpf_record (cpf_handle, ping_number, &record) != CZMIL_SUCCESS)
        {
          czmil_perror ();
          return (-1);
        }
    }


  prev_ping_number = ping_number;

  channel = beam_number / 100;
  retnum = beam_number % 100;

  if (validity & PFM_MANUALLY_INVAL) record.channel[channel][retnum].status = CZMIL_RETURN_MANUALLY_INVAL;
  if (validity & PFM_FILTER_INVAL) record.channel[channel][retnum].status = CZMIL_RETURN_FILTER_INVAL;
  if (validity & PFM_SUSPECT) record.channel[channel][retnum].status |= CZMIL_RETURN_SUSPECT;
  if (validity & CZMIL_RETURN_REPROCESSED) record.channel[channel][retnum].status |= CZMIL_RETURN_REPROCESSED;


  //  Set filter_reason to Hockey Puck filtered based on PFM_USER_06 set in czmilPfmFilter.

  if ((validity & PFM_FILTER_INVAL) && (validity & PFM_USER_06)) record.channel[channel][retnum].filter_reason = CZMIL_APP_HP_FILTERED;


  //  If the point is a water surface return we won't ever save the CZMIL_RETURN_REFERENCE flag.  Originally, water surface was going to be
  //  indicated by having an ip_rank value of 0.  Unfortunately, someone who shall remain nameless (but who looks exactly like Chris Macon)
  //  decided that they just had to be able to process a single waveform using both land and water processing modes (for areas along the edge of
  //  the water).  This caused us to have to use ip_rank to differentiate between land (1) and water (0) for hybrid processed returns.  So, the
  //  long and short of it is, ip_rank = 0 will still indicate water surface for older data that isn't hybrid processed.  HydroFusion will, from
  //  now on, assign a classification of 41 (water surface) or 42 (derived water surface) to water surface points.

  //  NEW INFORMATION - The CZMIL API now hard-codes classification to 41 when ip_rank = 0 and classification = 0, so we can dispense with
  //  checking ip_rank to determine water surface.

  if ((validity & PFM_REFERENCE) && (record.channel[channel][retnum].classification != 41 && record.channel[channel][retnum].classification != 42))
    record.channel[channel][retnum].status |= CZMIL_RETURN_REFERENCE;


  //  If the PFM record is marked as PFM_SELECTED_SOUNDING this means that the LiDAR classification has been modified.

  if (validity & PFM_SELECTED_SOUNDING)
    {
      record.channel[channel][retnum].status |= CZMIL_RETURN_CLASSIFICATION_MODIFIED;
      record.channel[channel][retnum].classification = cls;
    }
  if (validity & PFM_SELECTED_FEATURE) record.channel[channel][retnum].status |= CZMIL_RETURN_SELECTED_FEATURE;
  if (validity & PFM_DESIGNATED_SOUNDING) record.channel[channel][retnum].status |= CZMIL_RETURN_DESIGNATED_SOUNDING;


  return (0);
}



void check_las_file (char *path)
{
  static uint8_t lz_checked = NVFalse;


  //  Open the LAS file.

  LASreadOpener lasreadopener;
  LASreader *lasreader;

  lasreadopener.set_file_name (path);
  lasreader = lasreadopener.open ();
  if (!lasreader)
    {
      strcpy (out_str, QObject::tr ("\n\n*** ERROR ***\nUnable to open LAS file %1\n").arg (path).toUtf8 ());
      fprintf (stderr, "%s", out_str);
      fflush (stderr);
      return;
    }

  lasreader->close ();


  //  If we've got LAZ files, check for the laszip program.

  if (QString (path).endsWith (".laz") || QString (path).endsWith (".LAZ"))
    {
      if (!lz_checked && !lz_available)
        {
          char lz_name[1024];

#ifdef NVWIN3X
          strcpy (lz_name, "laszip.exe");
#else
          strcpy (lz_name, "laszip");
#endif
          if (find_startup_name (lz_name) != NULL) lz_available = NVTrue;

          lzName = QString (lz_name);

          if (!lz_available)
            {
              strcpy (out_str, QObject::tr ("\n\n*** ERROR ***\nLAZ files will not be unloaded because %1 is not in the PATH\n").arg (lz_name).toUtf8 ());
              fprintf (stderr, "%s", out_str);
              fflush (stderr);
            }

          lz_checked = NVTrue;
        }
    }
}



/********************************************************************
 *
 * Function Name : unload_las_file
 *
 * Description : Writes modified flags to a LAS file (just the
 *               "withheld" flag and/or the classification value).
 *
 ********************************************************************/

int32_t unload_las_file (int16_t file_number, int32_t ping_number, uint32_t validity, uint8_t cls, char *file, uint8_t *laz)
{
  static int16_t          prev_file_number = -1;
  static FILE             *las_fp;
  static LASheader        lasheader;
  static uint8_t          endian = 0;
  static char             prev_file[1024], current_file[1024];
  SLAS_POINT_DATA         slas;
  int32_t                 recnum = 0;


  //  If we've changed files close the old file and open the new one.

  if (file_number != prev_file_number)
    {
      if (file != NULL) strcpy (current_file, file);


      //  Clear the LAZ flag.

      *laz = NVFalse;


      //  Close old file.

      if (prev_file_number != -1) fclose (las_fp);


      //  If the previous LAS file was a compressed LAZ file we have to rename the old LAZ file to WHATEVER.bck, compress the new LAS file
      //  (created when we opened it - below) to the old LAZ file name, delete the LAS file, then delete the BCK file.  At that point we are back
      //  to the original LAZ file name.

      if (QString (prev_file).toUpper ().endsWith (".LAZ") && prev_file_number != -1)
        {
          if (lz_available)
            {
              QString prevFile = QString (prev_file);

              QFile bckFile (prevFile);

              QString prevFileBck = prevFile;

              prevFileBck.replace (".laz", ".bck");

              if (!bckFile.rename (prevFileBck))
                {
                  strcpy (out_str, QObject::tr ("\n\n*** ERROR ***\nUnable to rename LAZ file %1\n").arg (prev_file).toUtf8 ());
                  fprintf (stderr, "%s", out_str);
                  fflush (stderr);
                  return (-1);
                }


              QString prevFileLAS = prevFile;
              prevFileLAS.replace (".laz", ".las");


              QProcess zipper;
              QStringList zparams;

              zparams << prevFileLAS;

              zipper.start (lzName, zparams);

              zipper.waitForFinished (-1);


              QFile lasFile (prevFileLAS);

              if (!lasFile.remove ())
                {
                  char name[1024];
                  strcpy (name, prevFileLAS.toLatin1 ());

                  strcpy (out_str, QObject::tr ("\n\n*** ERROR ***\nUnable to remove LAS file %1\n").arg (name).toUtf8 ());
                  fprintf (stderr, "%s", out_str);
                  fflush (stderr);
                  return (-1);
                }


              if (!bckFile.remove ())
                {
                  char name[1024];
                  strcpy (name, prevFileBck.toLatin1 ());

                  strcpy (out_str, QObject::tr ("\n\n*** ERROR ***\nUnable to remove BCK file %1\n").arg (name).toUtf8 ());
                  fprintf (stderr, "%s", out_str);
                  fflush (stderr);
                  return (-1);
                }
            }


          //  Bail out if no more files.

          if (file_number < 0 && file == NULL) return (0);
        }


      //  If the current LAS file is a compressed LAZ file we have to uncompress it to a LAS file then update the LAS file.

      if (QString (file).toUpper ().endsWith (".LAZ"))
        {
          if (lz_available)
            {
              QProcess unzipper;
              QStringList uparams;

              uparams << QString (file);

              unzipper.start (lzName, uparams);

              unzipper.waitForFinished (-1);
            }
          else
            {
              strcpy (out_str, QObject::tr ("\n\n*** ERROR ***\nUnable to unload LAZ file %1\n").arg (file).toUtf8 ());
              fprintf (stderr, "%s", out_str);
              fflush (stderr);
              return (-1);
            }


          QString fileLAS = QString (file).replace (".laz", ".las");
          strcpy (current_file, fileLAS.toLatin1 ());


          //  Set the LAZ flag so we'll know if the last file was a .laz file.

          *laz = NVTrue;
        }


      //  Open the LAS file with LASlib and read the header.

      LASreadOpener lasreadopener;
      LASreader *lasreader;

      lasreadopener.set_file_name (file);
      lasreader = lasreadopener.open ();
      if (!lasreader)
        {
          strcpy (out_str, QObject::tr ("\n\n*** ERROR ***\nUnable to open LAS file %1\n").arg (file).toUtf8 ());
          fprintf (stderr, "%s", out_str);
          fflush (stderr);
          return (-1);
        }


      lasheader = lasreader->header;


      if (lasheader.version_major != 1)
        {
          lasreader->close ();
          strcpy (out_str, QObject::tr ("\nLAS major version %1 incorrect, file %2 : %3 %4 %5\n\n").arg
                  (lasheader.version_major).arg (file).arg (__FILE__).arg (__FUNCTION__).arg (__LINE__).toUtf8 ());
          fprintf (stderr, "%s", out_str);
          fflush (stderr);
          prev_file_number = -1;
          return (-1);
        }


      if (lasheader.version_minor > 4)
        {
          lasreader->close ();
          strcpy (out_str, QObject::tr ("\nLAS minor version %1 incorrect, file %2 : %3 %4 %5\n\n").arg
                  (lasheader.version_minor).arg (file).arg (__FILE__).arg (__FUNCTION__).arg (__LINE__).toUtf8 ());
          fprintf (stderr, "%s", out_str);
          fflush (stderr);
          prev_file_number = -1;
          return (-1);
        }


      //  Now close it since all we really wanted was the header.

      lasreader->close ();


      //  Check for endian-ness.

      endian = big_endian ();


      //  Open the file for update.

      if ((las_fp = fopen64 (current_file, "rb+")) == NULL)
        {
          strcpy (out_str, QObject::tr ("\nError opening LAS file %1 : %2\n\n").arg (current_file).arg (strerror (errno)).toUtf8 ());
          fprintf (stderr, "%s", out_str);
          fflush (stderr);
          prev_file_number = -1;
          return (-1);
        }

      last_fp = las_fp;
      last_type = PFM_LAS_DATA;

      prev_file_number = file_number;
      strcpy (prev_file, file);
    }


  recnum = ping_number - 1;


  if (slas_read_point_data (las_fp, recnum, &lasheader, endian, &slas))
    {
      strcpy (out_str, QObject::tr ("\nError %1 reading record %L2 in file %3 : %4 %5 %6\n\n").arg
              (strerror (errno)).arg (recnum).arg (current_file).arg (__FILE__).arg (__FUNCTION__).arg (__LINE__).toUtf8 ());
      fprintf (stderr, "%s", out_str);
      fflush (stderr);
      fclose (las_fp);
      return (-1);
    }
  else
    {
      //  Set the "synthetic" flag.

      if (validity & PFM_REFERENCE)
        {
          slas.synthetic = 1;
        }
      else
        {
          slas.synthetic = 0;
        }


      //  Set the "keypoint" flag.

      if (validity & PFM_SELECTED_FEATURE)
        {
          slas.keypoint = 1;
        }
      else
        {
          slas.keypoint = 0;
        }


      //  Set the "withheld" flag.

      if (validity & PFM_MANUALLY_INVAL)
        {
          slas.withheld = 1;
        }
      else
        {
          slas.withheld = 0;
        }


      //  If the PFM record is marked as PFM_SELECTED_SOUNDING this means that the LiDAR classification has been modified.

      if (validity & PFM_SELECTED_SOUNDING) slas.classification = cls;


      if (slas_update_point_data (las_fp, recnum, &lasheader, endian, &slas))
        {
          strcpy (out_str, QObject::tr ("\nError %1 updating record %L2 in file %3 : %4 %5 %6\n\n").arg
                  (strerror (errno)).arg (recnum).arg (current_file).arg (__FILE__).arg (__FUNCTION__).arg (__LINE__).toUtf8 ());
          fprintf (stderr, "%s", out_str);
          fflush (stderr);
          fclose (las_fp);
          exit (-1);
        }
    }

  return (0);
}



uint8_t check_llz_file (char *path)
{
  int32_t                 llz_hnd;
  LLZ_HEADER              llz_header;


  //  Open the llz file.

  if ((llz_hnd = open_llz (path, &llz_header)) < 0)
    {
      strcpy (out_str, QObject::tr ("\n\nUnable to open the LLZ file.\n%1 : %2\n").arg (path).arg (strerror (errno)).toUtf8 ());
      fprintf (stderr, "%s", out_str);
      fflush (stderr);
      return (NVFalse);
    }

  if (!strstr (llz_header.version, "llz library"))
    {
      strcpy (out_str, QObject::tr ("\n\nLLZ file version corrupt or %1 is not an LLZ file.\n").arg (path).toUtf8 ());
      fprintf (stderr, "%s", out_str);
      fflush (stderr);
      return (NVFalse);
    }

  close_llz (llz_hnd);

  return (NVTrue);
}



/********************************************************************
 *
 * Function Name : unload_llz_file
 *
 * Description : Writes modified flags to an llz file.  See llz.h for format.
 *
 ********************************************************************/

int32_t unload_llz_file (int16_t file_number, int32_t ping_number, uint32_t validity, char *file)
{
  static int32_t          llz_hnd = -1;
  static LLZ_HEADER       llz_header;
  LLZ_REC                 llz;
  static int16_t          prev_file_number = -1;


  //  If we've changed files close the old file and open the new one.

  if (file_number != prev_file_number)
    {

      //  Close old file.

      if (prev_file_number != -1) close_llz (llz_hnd);


      //  Open the llz file.

      if ((llz_hnd = open_llz (file, &llz_header)) < 0)
        {
          strcpy (out_str, QObject::tr ("\n\nError opening the LLZ file.\n%1 : %2\n").arg (file).arg (strerror (errno)).toUtf8 ());
          fprintf (stderr, "%s", out_str);
          prev_file_number = -1;
          fflush (stderr);
          return (-1);
        }


        last_handle = llz_hnd;
        last_type = PFM_NAVO_LLZ_DATA;


        prev_file_number = file_number;
    }


    read_llz (llz_hnd, ping_number, &llz);

    llz.status = validity & 0x00000003;


    update_llz (llz_hnd, ping_number, llz);

    return (0);
}



uint8_t check_dted_file (char *path)
{
  FILE           *fp;
  UHL            uhl;


  //  Open the DTED file.

  if ((fp = fopen (path, "rb")) == NULL)
    {
      strcpy (out_str, QObject::tr ("\n\nUnable to open the DTED file.\n%1 : %2\n").arg (path).arg (strerror (errno)).toUtf8 ());
      fprintf (stderr, "%s", out_str);
      return (NVFalse);
    }


  if (read_uhl (fp, &uhl) < 0)
    {
      strcpy (out_str, QObject::tr ("\n\n%1 is not a DTED file.\n").arg (path).toUtf8 ());
      fprintf (stderr, "%s", out_str);
      return (NVFalse);
    }


  fclose (fp);


  return (NVTrue);
}



/********************************************************************
 *
 * Function Name : unload_dted_file
 *
 * Description : This is a special unloader that writes a zero value
 *               back to the file if the point is set to invalid.  
 *               This is used to correct bad -32767 values only!
 *
 ********************************************************************/

int32_t unload_dted_file (int16_t file_number, int32_t ping_number, int16_t beam_number, uint32_t validity, char *file)
{
  static FILE             *fp = NULL;
  int32_t                 status;
  DTED_DATA               dted_data;
  static UHL              uhl;
  static int16_t          prev_file_number = -1;


  //  If we've changed files close the old file and open the new one.

  if (file_number != prev_file_number)
    {

      //  Close old file.

      if (prev_file_number != -1) fclose (fp);


      //  Open the dted file.

      if ((fp = fopen (file, "rb+")) == NULL)
        {
          strcpy (out_str, QObject::tr ("\nError opening the DTED file.\n%1 : %2\n").arg (file).arg (strerror (errno)).toUtf8 ());
          fprintf (stderr, "%s", out_str);
          prev_file_number = -1;
          fflush (stderr);
          return (-1);
        }

      read_uhl (fp, &uhl);


      last_fp = fp;
      last_type = PFM_DTED_DATA;


      prev_file_number = file_number;
    }


  status = read_dted_data (fp, uhl.num_lat_points, ping_number, &dted_data);

  if (status < 0)
    {
      strcpy (out_str, QObject::tr ("Unable to read DTED record at %L1\n").arg (ping_number).toUtf8 ());
      fprintf (stderr, "%s", out_str);
      fflush (stderr);
      return (-1);
    }

  if (validity & PFM_MANUALLY_INVAL) dted_data.elev[beam_number] = 0.0;

  status = write_dted_data (fp, uhl.num_lat_points, ping_number, dted_data);

  if (status < 0)
    {
      strcpy (out_str, QObject::tr ("Unable to write DTED record at %L1\n").arg (ping_number).toUtf8 ());
      fprintf (stderr, "%s", out_str);
      fflush (stderr);
      return (-1);
    }


  return (0);
}
