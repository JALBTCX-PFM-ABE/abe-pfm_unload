# PFM ABE - abe/pfm_unload

The PFM ABE suite of tools consists of many individual components that each carry a version number.  When put all together it is considered PFM ABE.The components can fall into 1 of 4 groups.
- ABE
- Utilities
- Libraries
- Open Source Libraries and tools

**pfm_unload** is part of the **abe** group.

## Version History

|App Version|Release Date|ABE Version|Notes|
|-------|------------|-----|---|
|V6.28|07/10/18|V7.0.0.0|JCD -Added code to save PFM_USER_06 in CZMIL data as CZMIL_APP_HP_FILTERED.  This flag is set by the czmilPfmFilter program.   |
|V6.30 | 11/21/19| |JCD - Wasn't clearing the CZMIL status prior to updating the status.  Because of this, re-validating wasn't working |
|V6.31|01/20/20|V7.0.0.1|JCD - Screwup with CZMIL_RETURN_REPROCESSED.  I wasn't checking PFM_USER_05, I was checking CZMIL_RETURN_REPROCESSED in the PFM status |



## Notes
