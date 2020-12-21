
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


/*********************************************************************************************

    This program is public domain software that was developed by 
    the U.S. Naval Oceanographic Office.

    This is a work of the US Government. In accordance with 17 USC 105,
    copyright protection is not available for any work of the US Government.

    This software is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

*********************************************************************************************/

#ifndef VERSION

#define     VERSION     "PFM Software - pfm_unload V6.31 - 01/20/20"

#endif

/*

    Version 1.0
    Jan C. Depner
    10/28/99


    Version 2.0
    Jan C. Depner
    02/15/00

    Went to version 2.0 of the PFM library.


    Version 2.1
    Jan C. Depner
    03/29/00

    Added SHOALS output.


    Version 2.2
    Jan C. Depner
    06/22/00

    Moved the history record write to after the file is closed.


    Version 2.3
    Jan C. Depner
    07/06/00

    Unloads merge format.


    Version 2.4
    Jan C. Depner
    09/04/00

    Replaced call to read_depth_record_index with read_depth_array_index.


    Version 2.5
    Jan C. Depner
    12/15/00

    Added undocumented "dump_all" feature.


    Version 2.6
    Jan C. Depner
    02/06/01

    After trying to unload an area that had over 2 billion soundings with 
    110 million edited I had to remove the sort.  It couldn't handle it.  So
    now it just opens and closes the files as it writes out the points.  It 
    will leave open a file until the file number changes though so it's not
    too bad.  There is a noticeable performance hit but it's not terrible
    since this is an unattended function.


    Version 2.61
    Jan C. Depner
    02/21/01

    Trying to catch all of the little bugs in the code.  Check for a handle
    if gsfOpen fails.  This is needed in case the index file is bad or can't
    be opened.


    Version 2.62
    Jan C. Depner
    05/03/01

    Remove the GSF index files before writing out changes.  This gets rid of
    any corrupted GSF index files and then regenerates them on open.  Don't
    know what was causing the corrupted GSF index files.  Apparently some
    other process.


    Version 2.63
    Jan C. Depner
    05/13/01

    Fixed a problem with the gettimeofday call in write_history.c.  Replaced
    it with a call to time.


    Version 2.64
    Jan C. Depner
    06/22/01

    Pass structure args to open_pfm_file.


    Version 3.0
    Jan C. Depner
    07/19/01
 
    4.0 PFM library changes.


    Version 3.1
    Jan C. Depner
    10/25/01
 
    Removed requirement for data to be marked as checked prior to unload.


    Version 3.2
    Jan C. Depner
    06/06/02
 
    Uses PFM_USER flags now (from PFM 4.1).  Maps the GSF flags correctly.


    Version 3.3
    Jan C. Depner
    11/18/02
 
    Added DEMO data type.


    Version 4.0
    Jan C. Depner
    06/12/03
 
    Added UNISIPS depth data type.  Added ability to handle reference masking.


    Version 4.1
    Jan C. Depner
    07/18/03
 
    Removed experimental reference masking and replaced with support for
    PFM_REFERENCE flag.  Went to 32 bits for validity fields.


    Version 4.11
    Jan C. Depner
    08/01/03
 
    Don't unload reference data.


    Version 4.2
    Jan C. Depner
    08/04/03
 
    Support for CHARTS HOF format.


    Version 4.3
    Jan C. Depner
    08/06/03
 
    Support for CHARTS TOF format.


    Version 4.4
    Jan C. Depner
    08/20/03
 
    Added partial unload to be shelled from pfm_view or pfm_edit.


    Version 4.5
    Jan C. Depner
    12/09/03
 
    Fixed computation of percentage on partial unload.


    Version 4.51
    Jan C. Depner
    05/19/04
 
    Changed undocumented unload all feature to -u option.


    Version 4.52
    Jan C. Depner
    10/27/04
 
    Wasn't setting the abdc to a negative for hof and/or tof.


    Version 4.6
    Jan C. Depner
    10/29/04
 
    Shifted PFM numbering of HOF and TOF to start at 1 so that we would be compatible
    with IVS.


    Version 4.61
    Jan C. Depner
    11/19/04
 
    Did the record shifting the correct way - in the libraries.


    Version 4.62
    Jan C. Depner
    12/16/04
 
    Changed Usage message for PFM 4.5 directory input.


    Version 4.63
    Jan C. Depner
    01/19/05
 
    Use |= instead of = when setting DELETED status in hof files.


    Version 4.64
    Jan C. Depner
    02/08/05
 
    Prliminary changes to support C++/Qt pfmView.


    Version 4.65
    Jan C. Depner
    02/25/05

    Switched to open_existing_pfm_file from open_pfm_file.


    Version 4.66
    Jan C. Depner
    03/04/05

    Fix return from open_existing_pfm_file.


    Version 4.67
    Jan C. Depner
    04/20/05

    Removed remnants of IVS licensing stuff.


    Version 4.68
    Jan C. Depner
    06/29/05

    Removed "Press enter to finish".


    Version 4.69
    Jan C. Depner
    10/26/05

    Changed usage for PFM 4.6 handle file use.


    Version 4.70
    Jan C. Depner
    04/20/06

    Removed "merge" file support.


    Version 4.71
    Jan C. Depner
    08/31/06

    Removed "demo" file support.  Added support for LLZ.


    Version 4.72
    Jan C. Depner
    01/17/07

    Invalidating TOF first returns if the last return is within 5 cm of the first return and the
    last return is invalid.  We don't load first returns in pfmLoad if they are within 5 cm
    of the last return but we want to make sure that they get invalidated on unload if they are
    essentially the same value as the last return.


    Version 4.73
    Jan C. Depner
    02/12/07

    Output error_on_update error message even when running from pfmView (Qt == NVTrue).


    Version 4.74
    Jan C. Depner
    02/14/07

    Don't close the last GSF file until we flush the buffer.


    Version 4.74
    Jan C. Depner
    09/17/07

    Replaced compute_index with compute_index_ptr.


    Version 4.75
    Jan C. Depner
    10/22/07

    Added fflush calls after prints to stderr since flush is not automatic in Windows.


    Version 4.77
    Jan C. Depner
    11/15/07

    Added support for writing zeros back to dted files when data is invalidated.


    Version 4.78
    Jan C. Depner
    03/28/08

    Inverted the nocheck option.  We've had fewer problems with corrupted GSF files lately
    so it's not really a neccesity anymore.  Made the -s and -a options undocumented since -a
    is only used from pfmView and -s is not really needed anymore (GSF corruption issue).


    Version 4.79
    Jan C. Depner
    04/07/08

    Replaced single .h files from utility library with include of nvutility.h


    Version 4.80
    Jan C. Depner
    06/05/08

    Cleaned up the dump_all option.  Since we're not resetting the modified flag in the depth
    record we don't need to check for invalid if we use the -u option.  Cleaned up indentation
    to match xemacs style.


    Version 4.81
    Jan C. Depner
    01/29/09

    Set checkpoint to 0 prior to calling open_existing_pfm_file.


    Version 4.82
    Jan C. Depner
    03/02/09

    Now handles WLF format data.


    Version 4.83
    Jan C. Depner
    06/15/09

    Added support for PFM_CHARTS_HOF_DATA.  Both primary and secondary returns are considered valid
    unless marked otherwise.


    Version 4.84
    Jan C. Depner
    06/25/10

    Now handles HAWKEYE format data.


    Version 4.85
    Jan C. Depner
    10/21/10

    Fix percent spinner when we're only unloading part of the area (like from pfmView).


    Version 4.86
    Jan C. Depner
    02/25/11

    Switched to using PFM_HAWKEYE_HYDRO_DATA and PFM_HAWKEYE_TOPO_DATA to replace the deprecated PFM_HAWKEYE_DATA.


    Version 4.87
    Jan C. Depner
    03/09/11

    Added a check for PFMWDB files so that we can try to unload them.  To unload them they must be moved to the PFMWDB
    directory and the unload operation run from there.  The easiest way to do this would be to use the find command
    like this:           find . -name \*.pfm -print -exec pfm_unload {} \;


    Version 4.90
    Jan C. Depner
    04/28/11

    Now adds 100 to abdc for HOF records that have been manually marked valid but were marked invalid (abdc < 70) by GCS.
    This way, pfmLoad will not automatically mark them as invalid (due to abdc < 70) if we build a new PFM.


    Version 5.00
    Jan C. Depner
    05/05/11

    Reads all modified records into memory and then sorts them by file and record number prior to unloading the edits to the
    input files.  This should save us some major disk thrashing on remote disks.  Keep in mind that if you have modified
    5 million records you will require 80MB of memory to do the unload.  On modern systems that shouldn't be a problem (and we
    usually fon't modify that many records).  Happy Cinco de Mayo!!!


    Version 5.01
    Jan C. Depner
    05/06/11

    Fixed problem with getopt that only happens on Windows.


    Version 5.02
    Jan C. Depner
    06/16/11

    Removed HMPS_SPARE_1 flag check since we don't do swath filtering anymore and it's been removed from hmpsflag.h.


    Version 5.03
    Jan C. Depner
    06/21/11

    Now override the AU_STATUS_DELETED_BIT in HOF records when the user declares data valid.


    Version 5.04
    Jan C. Depner
    08/03/11

    Will now error out if it can't write to the HOF or TOF file.


    Version 5.05
    Jan C. Depner
    07/14/12

    Now supports CZMIL preliminary formats.  Also, unloads reference data since CZMIL can support it.


    Version 5.06
    Jan C. Depner
    07/30/12

    - Cleaned up unload_file.c function calls (removed a bunch of unused attributes).
    - Changed unload_czmil_file to use czmil_update_cpf_return_status instead of czmil_update_cpf_record.
    - Made unload_czmil_file only update when the record changed (similar to GSF ping and beam handling).


    Version 5.07
    Jan C. Depner (PFM Software)
    06/24/13

    - Actually update the CZMIL return status on unload (novel idea, huh ;-)  This wasn't done before I left
      in November, 2012 because we didn't have a system that was ready for prime time.


    Version 5.08
    Jan C. Depner (PFM Software)
    02/26/14

    Cleaned up "Set but not used" variables that show up using the 4.8.2 version of gcc.


    Version 5.09
    Jan C. Depner (PFM Software)
    03/03/14

    Replaced HMPS flags with NV_GSF flags.


    Version 5.10
    Jan C. Depner (PFM Software)
    03/17/14

    Removed WLF support.  Top o' the mornin' to ye!


    Version 5.11
    Jan C. Depner (PFM Software)
    05/07/14

    Fixed format string problems.


    Version 5.12
    Jan C. Depner (PFM Software)
    05/27/14

    Removed UNISIPS support.


    Version 5.13
    Jan C. Depner (PFM Software)
    06/19/14

    - Removed PFMWDB support.  No one was using it.  It seemed like a good idea but I guess not.


    Version 5.20
    Jan C. Depner (PFM Software)
    07/12/14

    - Added support for LAS files (using the "witheld" bit in the classification).


    Version 5.21
    Jan C. Depner (PFM Software)
    07/17/14

    - No longer uses liblas.  Now uses libslas (my own API for LAS).


    Version 5.22
    Jan C. Depner (PFM Software)
    07/21/14

    - Removed support for old SHOALS files.  It's been gone from pfmLoad for years
      so I don't know why it was left in here.


    Version 5.23
    Jan C. Depner (PFM Software)
    07/23/14

    - Switched from using the old NV_INT64 and NV_U_INT32 type definitions to the C99 standard stdint.h and
      inttypes.h sized data types (e.g. int64_t and uint32_t).


    Version 6.00
    Jan C. Depner (PFM Software)
    12/27/14

    - "Ported" to C++.  Actually, all I really did was move a couple of things around and rename the files from
      .c to .cpp and .h to .hpp.  The only reason I did this was so that I could use std::vector.resize instead
      of realloc.  The realloc function is unbearably slow on Windows but std::vector.resize is just as fast as
      realloc on Linux and way faster on Windows.  In fact, using std::vector.resize is just about as fast on
      Windows as it is on Linux.


    Version 6.10
    Jan C. Depner (PFM Software)
    03/13/15

    - Replaced my libslas with rapidlasso GmbH LASlib and LASzip.  I still had to use part of libslas because LASlib won't update a record
      without rewriting the entire file.
    - Made preliminary file check always happen.


    Version 6.11
    Jan C. Depner (PFM Software)
    03/14/15

    - Since AHAB Hawkeye has switched to LAS format I have removed support for the old Hawkeye I binary format.


    Version 6.12
    Jan C. Depner (PFM Software)
    03/27/15

    - Added slas_update_point_data to slas.cpp and replaced the direct update of bit flags in the LAS record.
      It's a bit slower but much easier to use.


    Version 6.13
    Jan C. Depner (PFM Software)
    04/09/15

    - Modified slas.cpp to use extended_number_of_point_records for LAS v1.4 files.


    Version 6.14
    Jan C. Depner (PFM Software)
    04/24/16

    - Replaced CZMIL_RETURN_SLECTED_SOUNDING with CZMIL_RETURN_CLASSIFICATION_MODIFIED.


    Version 6.15
    Jan C. Depner (PFM Software)
    04/25/16

    - Now unloads classification changes to CZMIL and LAS files if the PFM_SELECTED_SOUNDING validity bit flag has been set.


    Version 6.16
    Jan C. Depner (PFM Software)
    04/29/16

    - Now unloads classification changes to CHARTS HOF and TOF files if the PFM_SELECTED_SOUNDING validity bit flag has been set.


    Version 6.17
    Jan C. Depner (PFM Software)
    05/05/16

    - Now unloads changes made to data that came from LAZ files.  Unfortunately we have to decompress the LAZ file, write the changes to
      the decompressed file, then re-compress it back to the LAZ file.  Hey, it works ;-)
    - Happy Cinco de Mayo!


    Version 6.18
    Jan C. Depner (PFM Software)
    05/10/16

    - Fixed incorrect error checking code when opening CPF file.


    Version 6.19
    Jan C. Depner (PFM Software)
    07/14/16

    - Fixed CZMIL unload bug.  DOH!


    Version 6.20
    Jan C. Depner (PFM Software)
    07/21/16

    - Don't save the CZMIL_RETURN_REFERENCE flag for water surface (ip_rank = 0) CZMIL returns.


    Version 6.21
    Jan C. Depner (PFM Software)
    03/21/17

    - Changed from C qsort to C++ std::sort.  This is much faster and should port better since it is actually a standard.


    Version 6.22
    Jan C. Depner (PFM Software)
    04/19/17

    - Save reference flag for CZMIL data when ip_rank = 0 and optech_classificiation = CZMIL_OPTECH_CLASS_HYBRID.  In that case
      the return is not necessarily a water surface point.


    Version 6.23
    Jan C. Depner (PFM Software)
    04/20/17

    -  Originally, water surface in CZMIL data was going to be indicated by having an ip_rank value of 0.  Unfortunately, someone,
       who shall remain nameless (but who looks exactly like Chris Macon) decided that they just had to be able to process a single
       waveform using both land and water processing modes (for areas along the edge of the water).  This caused us to have to use
       ip_rank to differentiate between land (1) and water (0) for hybrid processed returns.  So, the long and short of it is,
       ip_rank = 0 will still indicate water surface for older data but newer data (hybrid processed or not) will, from now on,
       have a classification of 41 (water surface) or 42 (derived water surface) for water surface points.  HydroFusion will assign
       the classification value.  This version of pfm_unload now checks for 41 and 42 in addition to ip_rank = 0 (for non-hybrid
       processed data) when deciding whether to save the reference flag for water surface data.


    Version 6.24
    Jan C. Depner (PFM Software)
    06/23/17

    -  Removed redundant functions from slas.cpp that are available in the nvutility library.


    Version 6.25
    Jan C. Depner (PFM Software)
    08/29/17

    - Now that the CZMIL API hard-codes ip_rank=0 points to classification 41 (where classification isn't already set), we
      no longer need to check ip_rank to determine water surface.


    Version 6.26
    Jan C. Depner (PFM Software)
    09/25/17

    - A bunch of changes to support doing translations in the future.  There is a generic
      pfm_unload_xx.ts file that can be run through Qt's "linguist" to translate to another language.


    Version 6.27
    Jan C. Depner (PFM Software)
    06/04/18

    - Replaced CZMIL_RETURN_USER_FLAG constant with CZMIL_RETURN_REPROCESSED due to change in CZMIL API.


    Version 6.28
    Jan C. Depner (PFM Software)
    07/10/19

    - Added code to save PFM_USER_06 in CZMIL data as CZMIL_APP_HP_FILTERED.  This flag is set by the 
      czmilPfmFilter program.


    Version 6.30
    Jan C. Depner (PFM Software)
    11/21/19

    - Wasn't clearing the CZMIL status prior to updating the status.  Because of this, re-validating
      wasn't working... DOH!!!


    Version 6.31
    Jan C. Depner (PFM Software)
    01/20/20

    - Screwup with CZMIL_RETURN_REPROCESSED.  I wasn't checking PFM_USER_05, I was checking CZMIL_RETURN_REPROCESSED in 
      the PFM status... MAJOR DOH!!!

*/
