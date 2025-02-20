/*
* Copyright (c)2015 - 2016 Oasis LMF Limited
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
*
*   * Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
*
*   * Redistributions in binary form must reproduce the above copyright
*     notice, this list of conditions and the following disclaimer in
*     the documentation and/or other materials provided with the
*     distribution.
*
*   * Neither the original author of this software nor the names of its
*     contributors may be used to endorse or promote products derived
*     from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
* COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
* OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
* AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
* THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*/
/*
Author: Ben Matharu  email: ben.matharu@oasislmf.org
*/

#ifndef AGGREPORTS_H_
#define AGGREPORTS_H_

#include <map>
#include <set>
#include <string>
#include <vector>
#include "leccalc.h"

#ifdef ORD_OUTPUT
#include "../include/oasisparquet.h"
#endif

enum { MEANDR = 1, FULL, PERSAMPLEMEAN, MEANSAMPLE };
enum { MEANS = 0, SAMPLES };
enum { WHEATSHEAF = 0, WHEATSHEAF_MEAN };

struct line_points{
	double from_x;
	double from_y;
	double to_x;
	double to_y;
};

class aggreports {
private:
	const int totalperiods_;
	std::set<int> summaryids_;
	std::vector<std::map<outkey2, OutLosses>> *out_loss_;
	FILE **fout_;
	bool useReturnPeriodFile_;
	int samplesize_;
	const bool *outputFlags_;
	const bool ordFlag_;
	const std::string *parquetFileNames_;
	bool eptHeader_ = true;
	bool pseptHeader_ = true;
	std::map<int, bool> wheatSheaf_ = {
		{ 0, true }, { OEP, false }, { AEP, false }
	};   // key 0 for fail safe
	std::vector<int> returnperiods_;
	std::map <int, double> periodstoweighting_;
	std::map<int, std::vector<int>> ensembletosidx_;
	std::map<int, bool> fileHeaders_ = {
		{ AGG_FULL_UNCERTAINTY, false }, { AGG_WHEATSHEAF, false },
		{ AGG_SAMPLE_MEAN, false }, { AGG_WHEATSHEAF_MEAN, false },
		{ OCC_FULL_UNCERTAINTY, false }, { OCC_WHEATSHEAF, false },
		{ OCC_SAMPLE_MEAN, false }, { OCC_WHEATSHEAF_MEAN, false }
	};
	void (aggreports::*WriteEPTOutput)(const std::vector<int>&, const int,
					   const int, const int, const double,
					   const OASIS_FLOAT) = nullptr;
	void (aggreports::*WritePSEPTOutput)(const std::vector<int>&, const int,
					     const int, const int, const double,
					     const OASIS_FLOAT) = nullptr;
#ifdef ORD_OUTPUT
	bool os_ept_exists_ = false;
	parquet::StreamWriter os_ept_;
	bool os_psept_exists_ = false;
	parquet::StreamWriter os_psept_;
#endif

	OASIS_FLOAT (OutLosses::*GetAgg)() = &OutLosses::GetAggOutLoss;
	OASIS_FLOAT (OutLosses::*GetMax)() = &OutLosses::GetMaxOutLoss;

	void LoadReturnPeriods();
	void LoadPeriodsToWeighting();
	void LoadEnsembleMapping();
	OASIS_FLOAT GetLoss(const double next_return_period,
			    const double last_return_period,
			    const OASIS_FLOAT last_loss,
			    const double current_return_period,
			    const OASIS_FLOAT current_loss) const;
	void FillTVaR(std::map<int, std::vector<TVaR>> &tail,
		      const int summary_id, const int epcalc,
		      const double nextreturnperiod_value,
		      const OASIS_FLOAT tvar);
	void FillTVaR(std::map<wheatkey, std::vector<TVaR>> &tail,
		      const int summary_id, const int sidx,
		      const double nextreturnperiod_value,
		      const OASIS_FLOAT tvar);
#ifdef ORD_OUTPUT
	template<typename T>
	void WriteReturnPeriodOut(const std::vector<int> &fileIDs,
		size_t &nextreturnperiod_index, double &last_return_period,
		OASIS_FLOAT &last_loss, const double current_return_period,
		const OASIS_FLOAT current_loss, const int summary_id,
		const int eptype, const int epcalc, const double max_retperiod,
		int counter, OASIS_FLOAT tvar, T &tail,
		void (aggreports::*WriteOutput)(const std::vector<int>&,
						const int, const int, const int,
						const double,
						const OASIS_FLOAT),
		parquet::StreamWriter& os);
#else
	template<typename T>
	void WriteReturnPeriodOut(const std::vector<int> &fileIDs,
		size_t &nextreturnperiod_index, double &last_return_period,
		OASIS_FLOAT &last_loss, const double current_return_period,
		const OASIS_FLOAT current_loss, const int summary_id,
		const int eptype, const int epcalc, const double max_retperiod,
		int counter, OASIS_FLOAT tvar, T &tail,
		void (aggreports::*WriteOutput)(const std::vector<int>&,
						const int, const int, const int,
						const double,
						const OASIS_FLOAT));
#endif
	inline void OutputRows(const std::vector<int> &fileIDs,
			       const char * buffer, int strLen);
	void WriteLegacyOutput(const std::vector<int> &fileIDs,
			       const int summary_id, const int type,
			       const int ensemble_id,
			       const double retperiod,
			       const OASIS_FLOAT loss);
	void WriteORDOutput(const std::vector<int> &fileIDs,
			    const int summary_id, const int epcalc,
			    const int eptype, const double retperiod,
			    const OASIS_FLOAT loss);
	void WriteTVaR(const std::vector<int> &fileIDs, const int epcalc,
		       const int eptype_tvar,
		       const std::map<int, std::vector<TVaR>> &tail);
	void WriteTVaR(const std::vector<int> &fileIDs, const int eptype_tvar,
		       const std::map<wheatkey, std::vector<TVaR>> &tail);
#ifdef ORD_OUTPUT
	inline void WriteParquetOutput(parquet::StreamWriter& os,
				       const int summary_id, const int epcalc,
				       const int eptype, const double retperiod,
				       const OASIS_FLOAT loss);
	inline void WriteTVaR(parquet::StreamWriter& os, const int epcalc,
			      const int eptype_tvar,
			      const std::map<int, std::vector<TVaR>> &tail);
	inline void WriteTVaR(parquet::StreamWriter& os, const int eptype_tvar,
			      const std::map<wheatkey, std::vector<TVaR>> &tail);
	inline parquet::StreamWriter GetParquetStreamWriter(const int fileStream);
#endif
	void WriteExceedanceProbabilityTable(const std::vector<int> &fileIDs,
					     std::map<int, lossvec> &items,
					     const double max_retperiod,
					     int epcalc, int eptype,
					     int eptype_tvar, int samplesize=1);
	void WriteExceedanceProbabilityTable(const std::vector<int> &fileIDs,
		std::map<int, lossvec2> &items,
		const OASIS_FLOAT cum_weight_constant, int epcalc, int eptype,
		int eptype_tvar,
		std::map<int, double> &unusedperiodstoweighting,
		int samplesize=1);
	void WritePerSampleExceedanceProbabilityTable(
		const std::vector<int> &fileIDs,
		std::map<wheatkey, lossvec> &items, int eptype,
		int eptype_tvar);
	void WritePerSampleExceedanceProbabilityTable(
		const std::vector<int> &fileIDs,
		std::map<wheatkey, lossvec2> &items, int eptype,
		int eptype_tvar,
		std::map<int, double> &unusedperiodstoweighting);
	void MeanDamageRatio(const std::vector<int> &fileIDs,
			     OASIS_FLOAT (OutLosses::*GetOutLoss)(),
			     const int epcalc, const int eptype,
			     const int eptype_tvar);
	void MeanDamageRatioWithWeighting(const std::vector<int> &fileIDs,
		OASIS_FLOAT (OutLosses::*GetOutLoss)(), const int epcalc,
		const int eptype, const int eptype_tvar);
	inline std::vector<int> GetFileIDs(const int handle, int table=EPT);
	void FullUncertainty(const std::vector<int> &fileIDs,
			     OASIS_FLOAT (OutLosses::*GetOutLoss)(),
			     const int epcalc, const int eptype,
			     const int eptype_tvar);
	void FullUncertaintyWithWeighting(const std::vector<int> &fileIDs,
		OASIS_FLOAT (OutLosses::*GetOutLoss)(), const int epcalc,
		const int eptype, const int eptype_tvar);
	inline void FillWheatsheafItems(const outkey2 key,
					std::map<wheatkey, lossvec> &items,
					const OASIS_FLOAT loss);
	inline void FillWheatsheafItems(const outkey2 key,
					std::map<wheatkey, lossvec2> &items,
					const OASIS_FLOAT loss,
					std::map<int, int> &maxPeriodNo,
					std::map<int, double> &unusedperiodstoweighting);
	void WheatsheafAndWheatsheafMean(const std::vector<int> handles,
					 OASIS_FLOAT (OutLosses::*GetOutLoss)(),
					 const int epcalc, const int eptype,
					 const int eptype_tvar,
					 int ensemble_id=0);
	void WheatsheafAndWheatsheafMeanWithWeighting(
		const std::vector<int> handles,
		OASIS_FLOAT (OutLosses::*GetOutLoss)(), const int epcalc,
		const int eptype, const int eptype_tvar, int ensemble_id=0);
	void SampleMean(const std::vector<int> &fileIDs,
			OASIS_FLOAT (OutLosses::*GetOutLoss)(),
			const int epcalc, const int eptype,
			const int eptype_tvar);
	void SampleMeanWithWeighting(const std::vector<int> &fileIDs,
				     OASIS_FLOAT (OutLosses::*GetOutLoss)(),
				     const int epcalc, const int eptype,
				     const int eptype_tvar);

public:
	aggreports(const int totalperiods, FILE **fout,
		   const bool useReturnPeriodFile, const bool *outputFlags,
		   const bool ordFlag, const std::string *parquetFileNames);
	void SetInputData(const std::set<int> &summaryids,
			  std::vector<std::map<outkey2, OutLosses>> &out_loss);
	void SetSampleSize(const int samplesize);
	void SetLegacyOutputFunc(int outputTable);
	void SetORDOutputFunc(int outputTable);
	void OutputMeanDamageRatio(int eptype, const int eptype_tvar,
				   OASIS_FLOAT (OutLosses::*GetOutLoss)(),
				   const std::vector<int> &fileIDs);
	void OutputFullUncertainty(const int handle, int eptype,
				   const int eptype_tvar,
				   OASIS_FLOAT (OutLosses::*GetOutLoss)());
	void OutputWheatsheafAndWheatsheafMean(const std::vector<int> &handles,
		int eptype, const int eptype_tvar,
		OASIS_FLOAT (OutLosses::*GetOutLoss)());
	void OutputSampleMean(const int handle, int eptype,
			      const int eptype_tvar,
			      OASIS_FLOAT (OutLosses::*GetOutLoss)());
};
#endif // AGGREPORTS_H_
