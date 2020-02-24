/* Copyright 2019 Fred Hutchinson Cancer Research Center
 * See the included LICENSE file for details 
 * on the licence that is granted to the user of this software.
 * CytoFrame.hpp
 *
 *  Created on: Sep 18, 2017
 *      Author: wjiang2
 */

#ifndef INST_INCLUDE_CYTOLIB_CYTOFRAME_HPP_
#define INST_INCLUDE_CYTOLIB_CYTOFRAME_HPP_
#include <armadillo>
using namespace arma;

#include "readFCSHeader.hpp"
#include "compensation.hpp"
#include <boost/lexical_cast.hpp>
#include <cytolib/global.hpp>
#include <unordered_map>

#include <H5Cpp.h>
using namespace H5;


namespace cytolib
{
enum class ColType {channel, marker, unknown};
enum class RangeType {instrument, data};
enum class FrameType {FCS, H5};
enum class H5Option {copy, move, skip, link, symlink};
enum DataTypeLocation {MEM, H5};
typedef unordered_map<string, string> PDATA;


const H5std_string  DATASET_NAME( "data");

/*
 * simple vector version of keyword type
 * used for communicate with h5 since h5 doesn't support customized container data type
 */
struct KEY_WORDS_SIMPLE{
			const char * key;
			const char * value;
			KEY_WORDS_SIMPLE(const char * k, const char * v):key(k),value(v){};
		};

class CytoFrame;
typedef shared_ptr<CytoFrame> CytoFramePtr;
struct KeyHash {
 std::size_t operator()(const string& k) const
 {
     return std::hash<std::string>()(boost::to_lower_copy(k));
 }
};
struct KeyEqual {
 bool operator()(const string& u, const string& v) const
 {
     return boost::to_lower_copy(u) == boost::to_lower_copy(v);
 }
};
typedef unordered_map<string, int, KeyHash, KeyEqual> PARAM_MAP;
/**
 * The class representing a single FCS file
 */
class CytoFrame{
protected:
	PDATA pheno_data_;
	KEY_WORDS keys_;//keyword pairs parsed from FCS Text section
	vector<cytoParam> params;// parameters coerced from keywords and computed from data for quick query
	PARAM_MAP channel_vs_idx;//hash map for query by channel
	PARAM_MAP marker_vs_idx;//hash map for query by marker

	CytoFrame (){};
	virtual bool is_hashed() const
	{
		return channel_vs_idx.size()==n_cols();
	}

	/**
	 * build the hash map for channel and marker for the faster query
	 *
	 */
	virtual void build_hash();
public:
	virtual ~CytoFrame(){};
//	virtual void close_h5() =0;
	CytoFrame(const CytoFrame & frm);

	CytoFrame & operator=(const CytoFrame & frm);

	CytoFrame & operator=(CytoFrame && frm);


	CytoFrame(CytoFrame && frm);


	compensation get_compensation(const string & key = "SPILL")
		{
			compensation comp;

			if(keys_.find(key)!=keys_.end())
			{
				string val = keys_[key];
				comp = compensation(val);
			}
			return comp;
		}


	virtual void convertToPb(pb::CytoFrame & fr_pb, const string & h5_filename, H5Option h5_opt) const = 0;

	virtual void set_readonly(bool flag){
	}
	virtual bool get_readonly(){
		return false;
		}
	virtual void compensate(const compensation & comp);

	virtual void scale_time_channel(string time_channel = "time");
	/**
	 * getter from cytoParam vector
	 * @return
	 */
	const vector<cytoParam> & get_params() const
	{
		return params;
	}
	virtual void set_params(const vector<cytoParam> & _params)
	{
		params = _params;
		build_hash();//update idx table

	}
//	virtual void writeFCS(const string & filename);

	FloatType h5_datatype_data(DataTypeLocation storage_type) const;
	CompType get_h5_datatype_params(DataTypeLocation storage_type) const;
	CompType get_h5_datatype_keys() const;
	virtual void write_h5_params(H5File file) const;
	/**
	 * Convert string to cstr in params for writing to h5
	 * @return
	 */
	vector<cytoParam_cstr> params_c_str() const;
	/**
	 * Convert string to cstr in keys/pdata for writing to h5
	 * @return
	 */
	template<class T>
	vector<KEY_WORDS_SIMPLE> to_kw_vec(const T & x) const{
		//convert to vector
		vector<KEY_WORDS_SIMPLE> keyVec;
		for(const auto & e : x)
		{
			keyVec.push_back(KEY_WORDS_SIMPLE(e.first.c_str(), e.second.c_str()));
		}
		return keyVec;
	}
	virtual void write_h5_keys(H5File file) const;
	virtual void write_h5_pheno_data(H5File file) const;
	/**
	 * save the CytoFrame as HDF5 format
	 *
	 * @param filename the path of the output H5 file
	 */
	virtual void write_h5(const string & filename) const;
	/**
	 * get the data of entire event matrix
	 * @return
	 */
	virtual EVENT_DATA_VEC get_data() const=0;
	virtual EVENT_DATA_VEC get_data(uvec col_idx) const=0;
	virtual EVENT_DATA_VEC get_data(vector<string>cols, ColType col_type) const
	{
		return get_data(get_col_idx(cols, col_type));
	}

	virtual void set_data(const EVENT_DATA_VEC &)=0;
	virtual void set_data(EVENT_DATA_VEC &&)=0;
	/**
	 * extract all the keyword pairs
	 *
	 * @return a vector of pairs of strings
	 */
	 virtual const KEY_WORDS & get_keywords() const{
		return keys_;
	}
	 virtual void set_keywords(const KEY_WORDS & keys){
			keys_ = keys;
		}
	/**
	 * extract the value of the single keyword by keyword name
	 *
	 * @param key keyword name
	 * @return keyword value as a string
	 */
	virtual string get_keyword(const string & key) const;

	/**
	 * set the value of the single keyword
	 * @param key keyword name
	 * @param value keyword value
	 */
	virtual void set_keyword(const string & key, const string & value)
	{
		keys_[key] = value;
	}

	/**
	 * get the number of columns(or parameters)
	 *
	 * @return
	 */
	virtual unsigned n_cols() const
	{
		return params.size();
	}

	/**
	 * get the number of rows(or events)
	 * @return
	 */
	virtual unsigned n_rows() const=0;

	virtual void subset_parameters(uvec col_idx);
	/**
	 * check if the hash map for channel and marker has been built
	 * @return
	 */

	/**
	 * get all the channel names
	 * @return
	 */
	virtual vector<string> get_channels() const;

	virtual void set_channels(const CHANNEL_MAP & chnl_map);
	/**
	 * Replace the entire channels
	 * It is to address the issue of rotating the original channels which
	 * can't be handled by one-by-one setter set_channel() API due to its duplication checks
	 * @param channels
	 */
	virtual void set_channels(const vector<string> & channels)
	{
		auto n1 = n_cols();
		auto n2 = channels.size();
		if(n2!=n1)
			throw(domain_error("The size of the input of 'set_channels' (" + to_string(n2) + ") is different from the original one (" + to_string(n1) + ")"));
		//duplication check
		set<string> tmp(channels.begin(), channels.end());
		if(tmp.size() < n1)
			throw(domain_error("The input of 'set_channels' has duplicates!"));
		for(unsigned i = 0; i < n1; i++)
		{
			params[i].channel = channels[i];
		}
		build_hash();
		//update_keywords
		for(unsigned i = 0; i < n1; i++)
		{
			auto kn = "$P" + to_string(i+1) + "N";
			set_keyword(kn, channels[i]);
		}


	}
	/**
	 * get all the marker names
	 * @return
	 */
	virtual vector<string> get_markers() const;
	/**
	 * Look up marker by channel info
	 * @param channel
	 * @return
	 */
	string get_marker(const string & channel);


	/**
	 * get the numeric index for the given column
	 * @param colname column name
	 * @param type the type of column
	 * @return
	 */
	virtual int get_col_idx(const string & colname, ColType type) const;
	uvec get_col_idx(vector<string> colnames, ColType col_type) const;
	virtual void set_channel(const string & oldname, const string &newname, bool is_update_keywords = true);

	virtual void set_marker(const string & channelname, const string & markername);

	/**
	 * Update the instrument range (typically after data transformation)
	 * @param colname
	 * @param ctype
	 * @param new_range
	 */
	virtual void set_range(const string & colname, ColType ctype, pair<EVENT_DATA_TYPE, EVENT_DATA_TYPE> new_range
			, bool is_update_keywords = true);
	/**
	 * the range of a specific column
	 * @param colname
	 * @param ctype the type of column
	 * @param rtype either RangeType::data or RangeType::instrument
	 * @return
	 */
	virtual pair<EVENT_DATA_TYPE, EVENT_DATA_TYPE> get_range(const string & colname
			, ColType ctype, RangeType rtype) const;
	/**
	 * Compute the time step from keyword either "$TIMESTEP" or "$BTIM", "$TIMESTEP" is preferred when it is present
	 * This is used to convert time channel to the meaningful units later on during data transforming
	 * @param
	 * @return
	 */
	EVENT_DATA_TYPE get_time_step(const string time_channel) const;

	virtual CytoFramePtr copy(const string & h5_filename = "", bool overwrite = false) const=0;
	virtual CytoFramePtr copy(uvec idx, bool is_row_indexed, const string & h5_filename = "", bool overwrite = false) const=0;
	virtual CytoFramePtr copy(uvec row_idx, uvec col_idx, const string & h5_filename = "", bool overwrite = false) const=0;
	
	
	virtual string get_h5_file_path() const=0;
	virtual void flush_meta(){};
	virtual void load_meta(){};

	const PDATA & get_pheno_data() const {return pheno_data_;}
	string get_pheno_data(const string & name) const ;
	virtual void set_pheno_data(const string & name, const string & value){
		pheno_data_[name] = value;
	}
	virtual void set_pheno_data(const PDATA & _pd)
	{
		pheno_data_ = _pd;
	}
	virtual void del_pheno_data(const string & name){
		pheno_data_.erase(name);}
};
};


#endif /* INST_INCLUDE_CYTOLIB_CYTOFRAME_HPP_ */
