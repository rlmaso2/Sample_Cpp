/* *
	ImageSequenceIO.cpp

	Authors: Ricky Mason(ricky.mason@uky.edu)
        Department of Electrical and Computer Engineering
		University of Kentucky
* */

#include <iostream>
#include <fstream>
#include <string>


#include <opencv2\opencv.hpp>

#include "Common.h"
#include "FileIO.h"

using namespace std;


namespace rm
{



	/******************************/
	/* The ImageSequenceIO class  */
	/******************************/
	/* *
		The class for writing and reading image sequences
	* */
	struct ImageSequenceIO::State
	{
	public:
		ImageSequenceIO			*m_pOwner;

		//IO streams
		ofstream				m_ofs;
		ifstream				m_ifs;
		//Image parameters
		ImageSequenceHeader		m_writeHeader;	//for writing
		ImageSequenceHeader		m_readHeader;	//for reading
		//Image data
		cv::Mat					m_readStreamImage;
		cv::Mat					m_processedImage;	//processed from read image
		int						m_readFrameId;

		int						m_bayerPattern;	//m_bayerPattern =-1 indicates no demosaicing
		
		//Parser Configuration
		string					m_readStreamFn;
		string					m_writeStreamFn;
		SequenceFileNames		m_writeFnManager;


	public:
		State(ImageSequenceIO *pOwner):m_pOwner(pOwner),m_bayerPattern(-1)
		{
			ResetWriteFns();
		}
		~State()
		{
			m_pOwner = NULL;
			if(m_ofs.is_open())
			{
				m_ofs.close();
			}
			if(m_ifs.is_open())
			{
				m_ifs.close();
			}
		}
		void ResetWriteFns()
		{
			if(m_writeFnManager.m_startIndex < 0)
			{
				m_writeFnManager.m_startIndex = 0;
			}
			if(m_writeFnManager.m_endIndex < 0)
			{
				m_writeFnManager.m_endIndex = MAX_INT;
			}
			m_writeFnManager.ResetCurrentIndex();
		}
		
		//
		//Parse the streams
		void ParseStream()
		{
			/*if(m_writeFnManager.m_startIndex < 0)
			{
				m_writeFnManager.m_startIndex = 0;
			}
			if(m_writeFnManager.m_endIndex < 0)
			{
				m_writeFnManager.m_endIndex = MAX_INT;
			}
			if(m_writeFnManager.m_indexStep > 0)
			{
				if(m_writeFnManager.m_currentIndex < m_writeFnManager.m_startIndex || 
					m_writeFnManager.m_currentIndex > m_writeFnManager.m_endIndex)
				{
					m_writeFnManager.m_currentIndex = m_writeFnManager.m_startIndex - m_writeFnManager.m_indexStep;
				}
			}
			else
			{
				if(m_writeFnManager.m_currentIndex > m_writeFnManager.m_startIndex || 
					m_writeFnManager.m_currentIndex < m_writeFnManager.m_endIndex)
				{
					m_writeFnManager.m_currentIndex = m_writeFnManager.m_startIndex - m_writeFnManager.m_indexStep;
				}
			}*/
			
			ResetWriteFns();
			m_pOwner->OpenReadStream(m_readStreamFn);
		
			cout<<"Saving Frame ";
			while(true)
			{
				//const cv::Mat &image = m_pOwner->ReadNextImage();
				if(m_pOwner->ReadNextImage() == -1)
				{
					cout<<"Finished! "<<m_writeFnManager.m_currentIndex + 1<<" has been read from the stream file!"<<endl;
					break;
				}
				m_pOwner->SaveCurrentReadFrame();
				cout<<m_writeFnManager.m_currentIndex<<"..";
			}

			m_writeFnManager.m_endIndex = m_writeFnManager.m_currentIndex;

			m_pOwner->CloseReadStream();
		}


		void SetBayerPattern(const string &pattern)
		{
			if(pattern == "RGGB")
			{
				m_bayerPattern = CV_BayerBG2BGR;				
			}
			else if(pattern == "GBRG")
			{
				m_bayerPattern = CV_BayerGR2BGR;
			}
			else if(pattern == "GRBG")
			{
				m_bayerPattern = CV_BayerGB2BGR;
			}
			else if(pattern == "BGGR")
			{
				m_bayerPattern = CV_BayerRG2BGR;
			}

		}
	};

	ImageSequenceIO::ImageSequenceIO()
	{
		m_pState = new ImageSequenceIO::State(this);
		if(!m_pState)
		{
			throw("ImageSequenceIO: failed to initialize, not enough memory");
		}
	}

	ImageSequenceIO::~ImageSequenceIO()
	{
		if(m_pState)
		{
			delete m_pState;
		}
	}
	
	//
	//Close the reading stream
	void ImageSequenceIO::CloseReadStream()
	{
		if(m_pState->m_ifs.is_open())
		{
			m_pState->m_ifs.close();
			m_pState->m_ifs.clear();
		}
	}
	//
	//Close the writing stream
	void ImageSequenceIO::CloseWriteStream()
	{
		if(m_pState->m_ofs.is_open())
		{
			m_pState->m_ofs.close();
			m_pState->m_ofs.clear();
		}
	}
	
	//
	//Get header information regarding the reading stream
	const ImageSequenceHeader& ImageSequenceIO::GetReadHeader() const
	{
		return m_pState->m_readHeader;
	}
	
	//
	//Get header information regarding the writing stream
	const ImageSequenceHeader& ImageSequenceIO::GetWriteHeader() const
	{
		return m_pState->m_writeHeader;
	}
	
	
	//
	void ImageSequenceIO::SetReadStreamSaveDataFileNames(const SequenceFileNames& sfns)
	{
		m_pState->m_writeFnManager = sfns;
		m_pState->m_writeFnManager.ResetCurrentIndex();
	}


	const SequenceFileNames& ImageSequenceIO::ReadStreamSaveDataFileNames() const
	{
		return m_pState->m_writeFnManager;
	}
	
	const std::string& ImageSequenceIO::ReadStreamFileName() const
	{
		return m_pState->m_readStreamFn;
	}

	const std::string& ImageSequenceIO::WriteStreamFileName() const
	{
		return m_pState->m_writeStreamFn;
	}

	//
	//Open a file stream for reading data and read the header information from the stream
	void ImageSequenceIO::OpenReadStream(const string &fileName)
	{
		CloseReadStream();
		if(&fileName != &m_pState->m_readStreamFn)
		{
			m_pState->m_readStreamFn = fileName;
		}
		m_pState->m_ifs.open(fileName,ios::in|ios::binary);
		if(!m_pState->m_ifs.is_open())
		{
			throw("ImageSequenceIO::OpenReadStream: failed to open the file stream");
		}

		ImageSequenceHeader &header = m_pState->m_readHeader;
		m_pState->m_ifs.read((char*)&(header.m_imaHeight),sizeof(int));
		m_pState->m_ifs.read((char*)&(header.m_imaWidth),sizeof(int));
		m_pState->m_ifs.read((char*)&(header.m_imaChannels),sizeof(int));
		m_pState->m_ifs.read((char*)&(header.m_imaBytesPerPixel),sizeof(int));

		//allocate space
		if(header.m_imaChannels == 3 && header.m_imaBytesPerPixel == 1)
		{//regular color image
			m_pState->m_readStreamImage.create(header.m_imaHeight,header.m_imaWidth,CV_8UC3);
		}
		else if(header.m_imaChannels == 1 && header.m_imaBytesPerPixel == 1)
		{//regular gray scale image
			m_pState->m_readStreamImage.create(header.m_imaHeight,header.m_imaWidth,CV_8U);
		}
		else if(header.m_imaChannels == 1 && header.m_imaBytesPerPixel == 2)
		{//16-bit image
			m_pState->m_readStreamImage.create(header.m_imaHeight,header.m_imaWidth,CV_16U);
		}
		else
		{
			throw("ImageSequenceIO::OpenReadStream: error in reading header - unknown image format");
		}
		if(m_pState->m_bayerPattern == -1)
		{
			m_pState->m_processedImage = m_pState->m_readStreamImage;	//just reference
		}
	}
	
	//
	//Open a file stream for writing data
	void ImageSequenceIO::OpenWriteStream(const string &fileName)
	{
		CloseWriteStream();
		m_pState->m_writeStreamFn = fileName;
		m_pState->m_ofs.open(fileName,ios::out|ios::binary);
		if(!m_pState->m_ofs.is_open())
		{
			throw("ImageSequenceIO::OpenWriteStream: failed to open the file stream");
		}
	}
	
	void ImageSequenceIO::ImportSettings(const std::string &configFn, const char* secName /*= "ImageSequenceIO"*/)
	{
		Settings settings(configFn);
		ImportSettings(settings,secName);
	}

	void ImageSequenceIO::ImportSettings(const Settings &settings, const char* secName /*= "ImageSequenceIO"*/)
	{
		try
		{
			m_pState->m_writeFnManager.ImportSettings(settings,secName);
			m_pState->ResetWriteFns();
		}
		catch(...)
		{
			//output settings could be missing, do nothing
		}
		double dSetting;
		string strSetting;
		settings.ReadSetting(secName,"streamFile",m_pState->m_readStreamFn,false);

		if(settings.ReadSetting(secName,"demosaic",strSetting,true))
		{
			m_pState->SetBayerPattern(strSetting);
		}
	}


	//
	//read the given stream file and parse them into individual files and save them in disk
	//
	void ImageSequenceIO::ParseStream()
	{	
		m_pState->ParseStream();
	}
	//use the FnSettings structure
	void ImageSequenceIO::ParseStream(const std::string &streamFn, const SequenceFileNames &fnManager)
	{
		m_pState->m_readStreamFn = streamFn;
		m_pState->m_writeFnManager = fnManager;
		m_pState->ParseStream();
	}

	//
	//Read the next image from the stream
	//If the end of file is reached, the return -1
	int ImageSequenceIO::ReadNextImage()
	{
		char *pImaData = (char*)(m_pState->m_readStreamImage.ptr());
		m_pState->m_ifs.read((char*)(&m_pState->m_readFrameId),sizeof(int));
		m_pState->m_ifs.read(pImaData,m_pState->m_readHeader.totalSize());
		if(m_pState->m_ifs.eof())
		{
			m_pState->m_readStreamImage.release();
			m_pState->m_processedImage.release();
			return -1;
		}
		if(m_pState->m_bayerPattern != -1)
		{
			cv::cvtColor(m_pState->m_readStreamImage,m_pState->m_processedImage,m_pState->m_bayerPattern);
		}
		return m_pState->m_readFrameId;
	}
	
	//last frame read through ReadNextImage
	const cv::Mat& ImageSequenceIO::LastReadFrame() const
	{
		return m_pState->m_processedImage;
	}

	const int ImageSequenceIO::LastReadFrameId() const
	{
		return m_pState->m_readFrameId;
	}

	void ImageSequenceIO::SaveCurrentReadFrame()
	{
		cv::imwrite(m_pState->m_writeFnManager.NextFileName(),m_pState->m_processedImage);
	}
	
	//
	//Set header for writing stream
	void ImageSequenceIO::SetWriteHeader(const ImageSequenceHeader &header)
	{
		m_pState->m_writeHeader = header;
	}

	//
	//Write the stream header
	void ImageSequenceIO::WriteHeader()
	{
		
		ImageSequenceHeader &header = m_pState->m_writeHeader;
#ifdef _DEBUG
		if(header.m_imaWidth==0 || header.m_imaHeight==0 || header.m_imaBytesPerPixel==0 || header.m_imaChannels==0)
		{
			throw("ImageSequenceIO::WriteHeader: header is not well defined");
		}
#endif
		m_pState->m_ofs.write((char*)&(header.m_imaHeight),sizeof(int));
		m_pState->m_ofs.write((char*)&(header.m_imaWidth),sizeof(int));
		m_pState->m_ofs.write((char*)&(header.m_imaChannels),sizeof(int));
		m_pState->m_ofs.write((char*)&(header.m_imaBytesPerPixel),sizeof(int));

	}

	//
	//Write an image to the stream
	void ImageSequenceIO::WriteImageToStream(const cv::Mat &image, const int frameId)
	{
		m_pState->m_ofs.write((char*)&m_pState->m_readFrameId,sizeof(int));
		m_pState->m_ofs.write((const char*)image.ptr(),m_pState->m_writeHeader.totalSize());
	}
	

}