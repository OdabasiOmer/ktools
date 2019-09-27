#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <set>
#include <utility>
#include <vector>
#include <stdio.h>
#include <stdlib.h>

#include "../include/oasis.h"

#if defined(_MSC_VER)
#include "../wingetopt/wingetopt.h"
#else
#include <unistd.h>
#endif

namespace validateoasisfiles {

  inline void SetPathToFile(char const * oasisFilesDir, char const * fileName,
			    char * pathToFile) {

    memcpy(pathToFile, oasisFilesDir, strlen(oasisFilesDir)+1);
    strncat(pathToFile, "/", (sizeof(pathToFile) - strlen(pathToFile)));
    strncat(pathToFile, fileName, (sizeof(pathToFile) - strlen(pathToFile)));

  }

  bool ReadFMProgrammeFile(char const * oasisFilesDir,
		  	   std::vector<int> &fromaggIDs,
			   std::set<std::pair<int, int> > &level_toaggIDPairs) {

    char line[4096];
    int lineno = 0;

    fm_programme fmprog;
    bool firstLevel = true;

    char const * fmprogrammeFileName = "fm_programme.csv";
    char fmprogrammePath[4096];
    SetPathToFile(oasisFilesDir, fmprogrammeFileName, fmprogrammePath);
    FILE * fmprogrammeFile;
    fmprogrammeFile = fopen(fmprogrammePath, "r");
    if(fmprogrammeFile == NULL) {
      fprintf(stderr, "Error opening %s\n", fmprogrammePath);
      return false;
    }
    fprintf(stderr, "Reading %s...\n", fmprogrammeFileName);

    fgets(line, sizeof(line), fmprogrammeFile);   // Skip header
    lineno++;
    fromaggIDs.clear();
    while(fgets(line, sizeof(line), fmprogrammeFile) != 0) {

      if(sscanf(line, "%d,%d,%d", &fmprog.from_agg_id, &fmprog.level_id,
		&fmprog.to_agg_id) != 3) {

	fprintf(stderr, "File %s\n", fmprogrammePath);
	fprintf(stderr, "Invalid data in line %d:\n%s\n", lineno, line);
	return false;

      } else {

	// Collect from_agg_id values for first level_id
	if(fmprog.level_id == 1) {

	  fromaggIDs.push_back(fmprog.from_agg_id);

	}

	// Collect unique (level_id, to_agg_id) pairs
	level_toaggIDPairs.insert(std::make_pair(fmprog.level_id,
						 fmprog.to_agg_id));

      }

      lineno++;

    }

    fclose(fmprogrammeFile);

    return true;

  }

  bool ReadCoveragesFile(char const * oasisFilesDir,
		  	 std::set<int> &coverageIDs) {

    char line[4096];
    int lineno = 0;

    int coverage_id;
    float tiv;

    char const * coveragesFileName = "coverages.csv";
    char coveragesPath[4096];
    SetPathToFile(oasisFilesDir, coveragesFileName, coveragesPath);
    FILE * coveragesFile;
    coveragesFile = fopen(coveragesPath, "r");
    if(coveragesFile == NULL) {
      fprintf(stderr, "Error opening %s\n", coveragesPath);
      return false;
    }
    fprintf(stderr, "Reading %s...\n", coveragesFileName);

    fgets(line, sizeof(line), coveragesFile);   // Skip header
    lineno++;
    coverageIDs.clear();
    while(fgets(line, sizeof(line), coveragesFile) != 0) {

      if(sscanf(line, "%d,%f", &coverage_id, &tiv) != 2) {
	
	fprintf(stderr, "File %s\n", coveragesPath);
	fprintf(stderr, "Invalid data in line %d:\n%s\n", lineno, line);
	return false;

      } else {

	// Collect coverage_id values
	coverageIDs.insert(coverage_id);

      }

      lineno++;

    }

    fclose(coveragesFile);

    return true;

  }

  bool ReadItemsFile(char const * oasisFilesDir,
		     std::set<int> const &coverageIDs,
		     std::vector<int> &fromaggIDs) {

    char line[4096];
    int lineno = 0;
    bool dataValid = true;

    item q;
 
    char const * itemsFileName = "items.csv";
    char itemsPath[4096];
    SetPathToFile(oasisFilesDir, itemsFileName, itemsPath);
    FILE * itemsFile;
    itemsFile = fopen(itemsPath, "r");
    if(itemsFile == NULL) {
      fprintf(stderr, "Error opening %s\n", itemsPath);
      return false;
    }
    fprintf(stderr, "Reading %s...\n", itemsFileName);

    fgets(line, sizeof(line), itemsFile);   // Skip header
    lineno++;
    while(fgets(line, sizeof(line), itemsFile) != 0) {

      if(sscanf(line, "%d,%d,%d,%d,%d", &q.id, &q.coverage_id, &q.areaperil_id,
		&q.vulnerability_id, &q.group_id) != 5) {

	fprintf(stderr, "File %s\n", itemsPath);
	fprintf(stderr, "Invalid data in line %d:\n%s\n", lineno, line);
	return false;

      } else {

	// Determine if coverage_id in items.csv matches those in coverages.csv
	if(coverageIDs.find(q.coverage_id) == coverageIDs.end()) {

	  fprintf(stderr, "Unknown coverage ID %d", q.coverage_id);
	  fprintf(stderr, " in items.csv on line %d\n%s\n", lineno, line);
	  dataValid = false;

	}

	// Determine if item_id in items.csv is present as from_agg_id in
	// fm_programme.csv
	std::vector<int>::iterator pos = std::find(fromaggIDs.begin(),
						   fromaggIDs.end(), q.id);
	if(pos != fromaggIDs.end()) {

	  fromaggIDs.erase(pos);

	} else {

	  fprintf(stderr, "Item ID %d in line %d", q.id, lineno);
	  fprintf(stderr, " not present in fm_programme.csv.");
	  fprintf(stderr, " Possible duplicate in items.csv?\n%s\n", line);
	  dataValid = false;

	}

      }

      lineno++;

    }

    fclose(itemsFile);

    // Display item_id in fm_programme.csv that is not present in items.csv
    if(!fromaggIDs.empty()) {

      fprintf(stderr, "Item ID(s) not found in items.csv");
      fprintf(stderr, " (possible duplicate(s) in fm_programme.csv):\n");
      for(std::vector<int>::const_iterator i=fromaggIDs.begin();
	  i!=fromaggIDs.end(); ++i) {
        fprintf(stderr, "%d ", *i);
      }
      fprintf(stderr, "\n");
      dataValid = false;

    }

    return dataValid;

  }

  bool ReadFMProfileFile(char const * oasisFilesDir,
		  	 std::set<int> &policytcIDs) {

    char line[4096];
    int lineno = 0;

    fm_profile fmprof;

    char const * fmprofileFileName = "fm_profile.csv";
    char fmprofilePath[4096];
    SetPathToFile(oasisFilesDir, fmprofileFileName, fmprofilePath);
    FILE * fmprofileFile;
    fmprofileFile = fopen(fmprofilePath, "r");
    if(fmprofileFile == NULL) {
      fprintf(stderr, "Error opening %s\n", fmprofilePath);
      return false;
    }
    fprintf(stderr, "Reading %s...\n", fmprofileFileName);

    fgets(line, sizeof(line), fmprofileFile);   // Skip header
    lineno++;
    while(fgets(line, sizeof(line), fmprofileFile) != 0) {

      if(sscanf(line, "%d,%d,%f,%f,%f,%f,%f,%f,%f,%f", &fmprof.profile_id,
		&fmprof.calcrule_id, &fmprof.deductible1, &fmprof.deductible2,
		&fmprof.deductible3, &fmprof.attachment, &fmprof.limit,
		&fmprof.share1, &fmprof.share2, &fmprof.share3) != 10) {

	fprintf(stderr, "File %s\n", fmprofilePath);
	fprintf(stderr, "Invalid data in line %d:\n%s\n", lineno, line);
	return false;

      } else {

	// Collect policytc_id values
	policytcIDs.insert(fmprof.profile_id);

      }

      lineno++;

    }

    fclose(fmprofileFile);

    return true;

  }

  bool ReadFMPolicyTCFile(char const * oasisFilesDir,
		  	  std::set<std::pair<int, int> > const &level_toaggIDPairs,
			  std::set<int> const &policytcIDs) {

    char line[4096];
    int lineno = 0;
    bool dataValid = true;

    fm_policyTC fmpol;

    char const * fmpolicytcFileName = "fm_policytc.csv";
    char fmpolicytcPath[4096];
    SetPathToFile(oasisFilesDir, fmpolicytcFileName, fmpolicytcPath);
    FILE * fmpolicytcFile;
    fmpolicytcFile = fopen(fmpolicytcPath, "r");
    if(fmpolicytcFile == NULL) {
      fprintf(stderr, "Error opening %s\n", fmpolicytcPath);
      return false;
    }
    fprintf(stderr, "Reading %s...\n", fmpolicytcFileName);

    fgets(line, sizeof(line), fmpolicytcFile);   // Skip header
    lineno++;
    while(fgets(line, sizeof(line), fmpolicytcFile) != 0) {

      if(sscanf(line, "%d,%d,%d,%d", &fmpol.layer_id, &fmpol.level_id,
		&fmpol.agg_id, &fmpol.profile_id) != 4) {

	fprintf(stderr, "File %s\n", fmpolicytcPath);
	fprintf(stderr, "Invalid data in line %d:\n%s\n", lineno, line);
	return false;

      } else {

	// Determine if (level_id, agg_id) pair in fm_policytc.csv is present
	// as (level_id, to_agg_id) pair in fm_programme.csv
	std::pair<int, int> level_toagg = std::make_pair(fmpol.level_id,
							 fmpol.agg_id);
	if(level_toaggIDPairs.find(level_toagg) == level_toaggIDPairs.end()) {

	  fprintf(stderr, "Unknown (level_id,agg_id)");
	  fprintf(stderr, " (%d,%d) pair in", fmpol.level_id, fmpol.agg_id);
	  fprintf(stderr, " fm_policytc.csv on line %d\n%s\n", lineno, line);
	  dataValid = false;

	}

	// Determine if policytc_id in fm_policytc.csv is present in
	// fm_profile.csv
	if(policytcIDs.find(fmpol.profile_id) == policytcIDs.end()) {

	  fprintf(stderr, "Unknown policytc_id %d", fmpol.profile_id);
	  fprintf(stderr, " in fm_policytc.csv on line %d\n%s\n", lineno, line);
	  dataValid = false;

	}

      }

      lineno++;

    }

    fclose(fmpolicytcFile);

    return dataValid;

  }

  void doit(char const * oasisFilesDir) {

    bool dataValid = true;
    std::vector<int> fromaggIDs;
    std::set<std::pair<int, int> > level_toaggIDPairs;
    std::set<int> coverageIDs;
    std::set<int> policytcIDs;

    fromaggIDs.clear();
    level_toaggIDPairs.clear();
    dataValid = ReadFMProgrammeFile(oasisFilesDir, fromaggIDs,
		    		    level_toaggIDPairs);
    if(dataValid == false) {
      return;
    } else {
      fprintf(stderr, "Done.\n");
    }

    coverageIDs.clear();
    dataValid = ReadCoveragesFile(oasisFilesDir, coverageIDs);
    if(dataValid == false) {
      return;
    } else {
      fprintf(stderr, "Done.\n");
    }

    dataValid = ReadItemsFile(oasisFilesDir, coverageIDs, fromaggIDs);
    if(dataValid == false) {
      return;
    } else {
      fprintf(stderr, "Done.\n");
    }

    policytcIDs.clear();
    dataValid = ReadFMProfileFile(oasisFilesDir, policytcIDs);
    if(dataValid == false) {
      return;
    } else {
      fprintf(stderr, "Done.\n");
    }

    dataValid = ReadFMPolicyTCFile(oasisFilesDir, level_toaggIDPairs,
		    		   policytcIDs);
    if(dataValid == false) {
      return;
    } else {
      fprintf(stderr, "Done.\n");
    }

    fprintf(stderr, "All checks pass.\n");
    return;

  }

}
