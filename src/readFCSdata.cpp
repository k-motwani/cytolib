/*
 * readFCSData.cpp
 *
 *  Created on: Sep 21, 2017
 *      Author: wjiang2
 */

#include "cytolib/readFCSdata.hpp"






//BYTES sortBytes(BYTES bytes,  unsigned nTotalBytes,std::vector<unsigned short> byte_order)
//{
//
//
//  unsigned elementSize = byte_order.size();
//
//  //how many element to return
//  auto nElement = nTotalBytes / elementSize ;
//  BYTES output(nTotalBytes);
//  for(auto ind = 0; ind < nElement; ind++){
//    for(auto i = 0; i < elementSize; i++){
//      auto j = byte_order.at(i);
//
//      auto pos_old = ind * elementSize + i;
//      auto pos_new = ind * elementSize + j;
////       if(ind<=10)
////         Rcpp::RPRINT( pos_old +":" + pos_new + std::endl;
//      output.at(pos_new) = bytes.at(pos_old);;
//    }
//
//  }
//  return output;
//
//}
/**
 * read data of single bitwidth
 * @param in
 * @param dattype
 * @param nBytes
 * @param size
 * @param isSigned
 * @param endian
 * @param splitInt
 * @param byte_order
 * @return
 */
unique_ptr<EVENT_DATA_TYPE> readFCSdataRaw(ifstream & in, string dattype
		, int nBytes, int size, bool isSigned, endianType endian, bool splitInt
		, string byte_order, bool isbyteswap){
	int count = nBytes/size;
	unique_ptr<EVENT_DATA_TYPE> output(new EVENT_DATA_TYPE[count]);
	EVENT_DATA_TYPE * data_ptr = output.get();
	vector<int>a = {1, 2, 4, 8};

	if(find(a.begin(),a.end(), size)!=a.end())
	{

		if(splitInt&&dattype == "I")
		{
		  if(size == 4)
		  {
			  throw(domain_error("split int not supported yet!"));
			//#reorder bytes for mixed endian
			if(endian == endianType::mixed)
			{

				throw(domain_error("mixed endian not supported yet!"));
//			  byte_order = as.integer(strsplit(byte_order, ",")[[1]]) - 1
//			  if(length(byte_order) != size)
//				stop("Byte order is not consistent with bidwidths!")
//
//			  bytes <- readBin(con=con, what="raw",n = nBytes,size=1)
//			  newBytes <- sortBytes(bytes, byte_order)#sort the bytes
//
//			  con <- newBytes
//			  endian <- "little"

			}

			//#read uint32 as two uint16
//			splitted <- readBin(con=con, what=dattype
//								,n = as.integer(count * 2) #coerce count again to ensure it is within the int limit
//								, size = size / 2, signed=FALSE, endian=endian)
//
//
//
//			uint2double(splitted, endian == "big")


		  }
		  else
			  throw(domain_error("'splitInt = TRUE' is only valid for uint32!"));
		}else
		{
			if(isbyteswap)
			{
				while(count-->0)
				{
					vector<BYTE> bytes(size);
					in.read(reinterpret_cast<char *>(&bytes[0]), size);
//					bytes2number(data_ptr, &bytes[0], size, isbyteswap);
					data_ptr++;
				}

			}
			else//directly load into dest
				in.read(reinterpret_cast<char *>(data_ptr), nBytes);



		}

	}
  else
	{
	  throw(domain_error("odd bitwidths not supported yet!"));
//			#read raw byte stream first
//			oldBytes <- readBin(con=con, what="raw",n = nBytes,size=1)
//			#convert to bit vector
//			oldBits<-rawToBits(oldBytes)
//			#convert the data element to the non-odd  bitwidth
//			oldBitWidth<-size*8
//			newBitWidth<-2^ceiling(log(oldBitWidth,2))
//			newBits<-unlist(lapply(1:count,function(i){
//	#							browser()
//												start<-(i-1)*oldBitWidth+1
//	                                            #padding zeros
//												c(oldBits[start:(start+oldBitWidth-1)],raw(newBitWidth-oldBitWidth)
//	                                                )
//												}
//									)
//							)
//			#convert raw byte to corresponding type by readBin
//	        #packBits is least-significant bit first, so we need to make sure endian is set to "little" instead of the endian used in original FCS
//			readBin(packBits(newBits,"raw"),what=dattype,n=count,size=newBitWidth/8, signed=signed, endian = "little")
//
//
		}
	return output;
}
//void convertRaw_impl(char * src, void * dest, unsigned short thisSize, bool isbyteswap)
//{
//
//  char * p = reinterpret_cast<char *>(dest);
//  memcpy(dest, src, thisSize);
//  if(isbyteswap)
//	  std::reverse(p, p + thisSize);
//
//}
EVENT_DATA_PTR readFCSdata(ifstream &in, const FCS_Header & header,KEY_WORDS & keys, vector<cytoParam> & params, int &nEvents,  FCS_READ_DATA_PARAM & config)
{
	//## transform or scale data?
	  bool fcsPnGtransform = false, isTransformation, scale;
	  if(config.transform == TransformType::linearize)
	  {
		isTransformation =  true;
		scale = false;
	  } else if (config.transform == TransformType::scale) {
		  isTransformation = true;
		scale = true;
	  } else if (config.transform == TransformType::linearize_with_PnG_scaling) {
		  isTransformation =  true;
			scale = false;
		fcsPnGtransform = true;
	  } else if (config.transform == TransformType::none) {
		  isTransformation =  false;
		scale = false;
	  }

	  if (fcsPnGtransform)
		  keys["flowCore_fcsPnGtransform"] = "linearize-with-PnG-scaling";
	  bool transDefinedinKeys = keys.find("transformation")!=keys.end();
	 if(transDefinedinKeys)
		 if(keys["transformation"] == "applied"||keys["transformation"] ==  "custom")
			 isTransformation =  false;


	string byte_order = keys["$BYTEORD"];
	endianType endian;
	if(byte_order == "4,3,2,1" || byte_order == "2,1")
		endian = endianType::big;
	else if(byte_order == "1,2,3,4" || byte_order == "1,2")
		endian = endianType::small;
	else
		endian = endianType::mixed;

	bool isbyteswap = false;
	if((is_host_big_endian()&&endian==endianType::small)||(!is_host_big_endian()&&endian==endianType::big))
		isbyteswap = true;


	 string dattype = keys["$DATATYPE"];
	 if(dattype!="I"&&dattype!="F"&&dattype!="D")
		 throw(domain_error("Don't know how to deal with $DATATYPE"));


    if (keys["$MODE"] != "L")
    	throw(domain_error("Don't know how to deal with $MODE" + keys["$MODE"]));


	fcsPnGtransform = keys.find("flowCore_fcsPnGtransform")!= keys.end() && keys["flowCore_fcsPnGtransform"] == "linearize-with-PnG-scaling";

//	int nrowTotal= boost::lexical_cast<int>(keys["$TOT"]);
	int nCol = params.size();

	bool multiSize = false;
	for(int i = 1; i < nCol; i++)
		if(params[i].PnB!=params[0].PnB)
		{
			multiSize = true;
			break;
		}
	if(dattype!="I"&&multiSize)
		throw(domain_error("Sorry, Numeric data type expects the same bitwidth for all parameters!"));

	bool splitInt;
	if(dattype=="I"){

//	  vector<unsigned> range(nCol);
//
//	  for(int i = 1; i <= nCol; i++)
//		  range[i-1] = boost::lexical_cast<unsigned>(range_str[i-1]);
	  if(multiSize)
		splitInt = false;
	  else
		  splitInt = params[0].PnB == 32;


	}
	else
	{
	  splitInt = false;

	}



	if(!multiSize){
	  if(params[0].PnB ==10){
		  string sys = keys["$SYS"];
		  transform(sys.begin(), sys.end(), sys.begin(),::tolower);
		if(sys !=  "cxp")
		  PRINT("Invalid bitwidth specification.\nThis is a known bug in Beckman Coulter's CPX software.\nThe data might be corrupted if produced by another software.\n");
		else
			PRINT("Beckma Coulter CPX data.\nCorrected for invalid bitwidth 10.\n");

		for(auto &p : params)
			p.PnB = 16;
	  }
	}

	bool isSigned;
	if(multiSize){

	  isSigned = false; // #dummy. not used in mutliSize logic.
	}else{


	  //# since signed = FALSE is not supported by readBin when size > 2
	  //# we set it to TRUE automatically then to avoid warning flooded by readBin
	  //# It shouldn't cause data clipping since we haven't found any use case where datatype is unsigned integer with size > 16bits
	 isSigned = !(params[0].PnB == 8 ||params[0].PnB == 16);
	}



  in.seekg(header.datastart);

  int nBytes = header.dataend - header.datastart + 1;

//  if(multiSize){
//	if(endian == endianType::mixed)
//		throw(domain_error("Cant't handle diverse bitwidths while endian is mixed: " + byte_order));

//	vector<BYTE>bytes(nBytes);
//	in.read((char *)&bytes[0], nBytes);
//
//	if(dattype != "i")
//	  throw(domain_error("we don't support different bitwdiths for numeric data type!"));
	//total bits for each row
  	size_t nRowSize = accumulate(params.begin(), params.end(), 0, [](size_t i, cytoParam p){return i + p.PnB;});

	//how many rows
//  	KEY_WORDS::iterator it = keys.find("$TOT");
//  	if(it==keys.end())
//  		throw(domain_error("can't find $TOT keyword to process the parsing!"));
//  	string sTot = it->second;
//
//	unsigned nrow = boost::lexical_cast<unsigned>(sTot);
  	unsigned nrow = nBytes * 8/nRowSize;
	nEvents = nrow;
	//how many element to return
	auto nElement = nrow * nCol;
	EVENT_DATA_PTR output(new EVENT_DATA_TYPE[nElement]);

	//load entire data section with one disk IO
	unique_ptr<char []> buf(new char[nBytes]);//we need to rearrange dat from row-major to col-major thus need a separate buf anyway (even for float)
	char * bufPtr = buf.get();
	in.read(bufPtr, nBytes); //load the bytes from file
//	char *p = buf.get();//pointer to the current beginning byte location of the processing data element in the byte stream
	float decade = pow(10, config.decades);
	/**
	 * cp raw bytes(row-major) to a 2d mat (col-major) represented as 1d array(with different byte width for each elements)
	 */
//	double start = omp_get_wtime();//clock();
#ifdef _OPENMP
	omp_set_num_threads(config.num_threads);
#endif

    #pragma omp parallel for
	for(auto c = 0; c < nCol; c++)
	 {
		size_t element_offset = nrow * c;
		size_t bits_offset = accumulate(params.begin(), params.begin() + c, 0, [](size_t i, cytoParam p){return i + p.PnB;});
		for(auto r = 0; r < nrow; r++)
	    {
	      //convert each element
			  cytoParam & param = params[c];

			  auto thisSize = param.PnB;
			  size_t idx = element_offset + r;
			  EVENT_DATA_TYPE & outElement = output[idx];
			  size_t idx_bits = r * nRowSize + bits_offset;
			  char *p = bufPtr + idx_bits/8;
			  unique_ptr<char []> tmp;
			  if(thisSize%8)//deal with odd bitwidth
			  {
				  if(thisSize>sizeof(uint64_t)*8)
				  {
					  std::string serror = "odd bitwidths exceeds the data type limit:";
						serror.append(std::to_string(thisSize));
						throw std::range_error(serror.c_str());
				  }
				  else
				  {

					  //cp the source bits
					  int nSizeBytes = (float)thisSize/8 + 1;
					  tmp.reset(new char[nSizeBytes]);
					  memcpy(tmp.get(), p, nSizeBytes);
					  //unset the extra bits for the trailing byte
					  int extraBits = nSizeBytes * 8 - thisSize;
					  tmp[nSizeBytes-1] &= (255<<extraBits);
					  p = tmp.get();
					  thisSize = nSizeBytes;

				  }

			  }
			  else
				  thisSize/=8;
			  if(isbyteswap)
				  std::reverse(p, p + thisSize);

			  if(dattype == "I")
			  {
				  switch(thisSize)
				  {
				  case sizeof(BYTE)://1 byte
					{
					  outElement = static_cast<EVENT_DATA_TYPE>(*p);
					}

					  break;
				  case sizeof(unsigned short): //2 bytes
					{

					  outElement = static_cast<EVENT_DATA_TYPE>(*reinterpret_cast<unsigned short *>(p));
					}

					break;
				  case sizeof(unsigned)://4 bytes
					{
					  outElement = static_cast<EVENT_DATA_TYPE>(*reinterpret_cast<unsigned *>(p));
					}

					break;
				  case sizeof(uint64_t)://8 bytes
					{
					  outElement = static_cast<EVENT_DATA_TYPE>(*reinterpret_cast<uint64_t *>(p));
					}

				  break;
				  default:
					  {
						  std::string serror = "unsupported byte width :";
						  serror.append(std::to_string(thisSize));
						  throw std::range_error(serror.c_str());
					  }
				  }
				  // apply bitmask for integer data

				 if(param.max > 0)
				 {
				   int usedBits = ceil(log2(param.max));
				   if(usedBits < param.PnB)
					   outElement = static_cast<int>(outElement) % (1<<usedBits);
				}


			}
			else
			{
			  switch(thisSize)
			  {
			  case sizeof(float):
				{
				  outElement = *reinterpret_cast<float *>(p);
				}

				break;
			  case sizeof(double):
				{
				  outElement = static_cast<EVENT_DATA_TYPE>(*reinterpret_cast<double *>(p));
				}

				break;
			  default:
				std::string serror ="Unsupported bitwidths for numerical data type:";
				serror.append(std::to_string(thisSize));
				throw std::range_error(serror.c_str());
			  }
			}

		  // truncate data at range
			if(!transDefinedinKeys)
			{
				if(config.truncate_max_range&&outElement > param.max)
					outElement = param.max;

				if(config.truncate_min_val&&outElement < config.min_limit)
					outElement = config.min_limit;
			}



//				## Transform or scale if necessary
//				# J.Spidlen, Nov 13, 2013: added the flowCore_fcsPnGtransform keyword, which is
//				# set to "linearize-with-PnG-scaling" when transformation="linearize-with-PnG-scaling"
//				# in read.FCS(). This does linearization for log-stored parameters and also division by
//				# gain ($PnG value) for linearly stored parameters. This is how the channel-to-scale
//				# transformation should be done according to the FCS specification (and according to
//				# Gating-ML 2.0), but lots of software tools are ignoring the $PnG division. I added it
//				# so that it is only done when specifically asked for so that read.FCS remains backwards
//				# compatible with previous versions.


			if(isTransformation)
			{


			  if(param.PnE.first > 0)
			  {
	//				 # J.Spidlen, Nov 5, 2013: This was a very minor bug. The linearization transformation
	//				 # for $PnE != "0,0" is defined as:
	//				 # For $PnR/r/, r>0, $PnE/f,0/, f>0: n is a logarithmic parameter with channel values
	//				 # from 0 to r-1. A channel value xc is converted to a scale value xs as xs=10^(f*xc/r).
	//				 # Note the "r" instead of the "r-1" in the formula (which would admitedly make more sense)
	//				 # However, this is the standard that apparently has been followed by BD and other companies
	//				 # "forever" and it is therefore addoped as such by the ISAC DSTF (see FCS 3.1 specification)
	//				 # To bring this to compliance, I am just changing
	//				 # dat[,i] <- 10^((dat[,i]/(range[i]-1))*ampli[i,1])
	//				 # to
	//				 # dat[,i] <- 10^((dat[,i]/range[i])*ampli[i,1])
				 outElement = pow(10,((outElement/param.max)*param.PnE.first));
//				 param.max = pow(10,param.PnE.first);
			  }
			  else if (fcsPnGtransform && param.PnG != 1) {
				  outElement = outElement / param.PnG;
//				  param.max = (param.max-1) / param.PnG;
			  }
//			  else
//				  param.max--;
			}
			if(scale)
			{
				if(param.PnE.first > 0)
				{
					outElement = decade*((outElement-1)/(param.max-1));
//					param.max = decade*(param.max/param.max-1);
				}
				else
				{
					outElement = decade*((outElement)/(param.max));
//					param.max = decade;
				}
			}

	    }
	 }
//	 cout << (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000) << endl;
//	 cout << (omp_get_wtime() - start) / (double)(CLOCKS_PER_SEC / 1000) << endl;

	//update params
	for(auto &p : params)
	{

		if(isTransformation)
		{


		  if(p.PnE.first > 0)
			 p.max = pow(10,p.PnE.first);
		  else if (fcsPnGtransform && p.PnG != 1)
			  p.max = (p.max-1) / p.PnG;
		  else
			  p.max--;
		}
		if(scale)
		{
			if(p.PnE.first > 0)
				p.max = decade*(p.max/p.max-1);
			else
				p.max = decade;

		}


	}
	config.isTransformed = isTransformation;
	return(output);

}

