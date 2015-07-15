/* *
	Camera.h		

	Authors: Ricky Mason(ricky.mason@uky.edu)
        Department of Electrical and Computer Engineering
		University of Kentucky
* */



#ifndef CAMERA_H_
#define CAMERA_H_


#include <string>
#include <vector>
#include <math.h>

#include "Common.h"

#include "CameraSettings.h"

// forward declaration
namespace cv
{
	class Mat;
};



namespace rm
{
	
	/************************************************************//**
	 *	The abstract Camera class  
	 *	This is the common interface for all camera class
	 ***************************************************************/
	class Camera
	{		
	public:
			
		
		/** \brief Perform one capture (could be multiple images)	
		 */
		virtual void GrabOne() = 0;
			
		/** \brief Get the file extenstion for saving the image	
		 *	\param[in] index The index of the internal camera(or image), default=0
		 */
		virtual const std::string& FileNameExtension(const int index = 0) const = 0;

		/** \brief Y resolution
		 *	\param[in] index The index of the internal camera(or image), default=0
		 */
		virtual int Height(const int index = 0) const = 0;

		/** \brief X resolution
		 *	\param[in] index The index of the internal camera(or image), default=0
		 */
		virtual int Width(const int index = 0) const = 0;

		/** \brief set resolution
		 */
		//virtual	void setResolution(const int &height, const int &width, const int &index = 0) = 0;

		/** \brief frameRate
		 *	\param[in] index The index of the internal camera(or image), default=0
		 */
		virtual float FrameRate(const int index = 0) const  = 0;


		/** \brief trigger mode
		 *	\param[in] index The index of the internal camera(or image), default=0
		 */
		virtual int TriggerMode(const int index = 0) const  = 0; 


		/** \brief Set camera configuartion
		 *	\param[in] camSettings The cameras settings
		 *	\param[in] index [optional] The index of camera in this device if there are multiple ones.
		 */
		virtual void ConfigCamera(const CameraSettings &camSettings, const int index = 0) = 0;

		
		/** \brief Channels
		 *	\param[in] index The index of the internal camera(or image), default=0
		 */
		virtual int Channels(const int index = 0) const = 0;
	
		
		/** \brief Bytes per pixels
		 *	\param[in] index The index of the internal camera(or image), default=0
		 */
		virtual int BytesPerPixel(const int index = 0) const = 0;

		/** \brief Check if visualization of the indicated image is enabled 
		 */
		virtual bool IsVizEnabled(const int index = 0) const = 0;

		/** \brief Retrieve images for visualization purpose, could be different from the raw data
		 *	\param[out] vizIma The image to save the retrieved data.
		 *	\param[in] index The index of the internal camera(or image), default=0		 
		 */
		virtual void GetVizImage(cv::Mat &vizIma, const int index = 0) const = 0 ;
	
		/** \brief Set locks to all the images internally, so that no update will be performed
		 *	Usually, lock is set and then ALL desired images are retrieved and then lock is released
		 *	\return False if error occurs during setting the lock
		 */
		virtual bool SetLock() const = 0;

		/** \brief Retrieve images (by default retrieve the first image)
		 *	NOTE: the releaseImage should be called when using of the retrieved image through 
		 *	this function is finished, so that update could be performed when new data comes.
		 *  Normally, the data obtained from this function is retrieved, and then copied and finally released.
		 *	\param[in] index The index of the internal camera(or image), default=0
		 */
		virtual const cv::Mat& GetImage(const int index = 0) const = 0;

		
		/** \brief Release the lock previousely set
		 *	This function is usually called after using getImages to retrieve images
		 *	\return False if error occurs during releasing the lock
		 */
		virtual bool ReleaseLock() const = 0;

		/** \brief Return the number of images per capture.
		 *	\return The number of images captured
		 */
		virtual int NumImages() const = 0 ;

		/** \brief Return the number of images captured by this camera for visualization purpose
		 *	\return The number of images for visualization
		 */
		//virtual int numVizImages() const = 0;

		//
		//Return a set of names for each of the image
		//virtual const std::vector<std::string>& imageNames() const = 0;

		/** \brief Return the image name for the image indicated by the index
		 *	\param[in] index The index of the image captured by the camera
		 */
		virtual const std::string& ImageName(const int index = 0) const = 0;
	
		/** \brief Read settings from a configuration file
		 *	\param[in] fn The configuration file name
		 *	\param[in] secName The section name in the config file that corresponds to this camera
		 *				This will be used for some cameras, but not the others.
		 */
		virtual void ImportSettings(const std::string &fn, const char *secName = "Camera") = 0;
		/** \brief Read settings from a Settings struct
		 *	\param[in] settings The configuration structure
		 *	\param[in] secName The section name in the config file that corresponds to this camera
		 *				This will be used for some cameras, but not the others.
		 */
		virtual void ImportSettings(const Settings &settings, const char *secName = "Camera") = 0;

		/** \brief Initiate the camera
		 *	\param[in] pData Possible data needed for initialization
		 */
		virtual int Init(void* pData = NULL) = 0;

		/** \brief Start grabbing
		 */
		virtual void StartGrab() = 0;

		/** \brief Set the directory for data saving
		 *	\param[in] saveDir The target directory for saving data
		 *	\param[in] prefix If multiple instance (cameras) of the same class are running,
		 *	this will help tell them apart.
		 */
		virtual void SetSavePath(const std::string &saveDir,const std::string &prefix) = 0;

		/** \brief Save all captured data
		 *	\param[in] frameId The current frame index
		 *	\param[in] streamId The data saving has two modes: saving each frame as single files
		 *	or saving the sequence as single stream files. If streamId is set to -1 (default),
		 *	then the function will do the former; otherwise it would do the latter and use streamId
		 *	as the stream file index
		 */
		virtual void SaveData(const int frameId, const int streamId = -1) = 0;


		//virtual void SaveData(const std::string &saveDir, const std::string &prefix, const int frameId, const char *fmt = "%s%s%s%04d%s") = 0;
		

		/**	\brief Stop the camera
		 */
		virtual void ShutDown() = 0;
	};

	
	
	/**********************************************************************/
	//	The common depth and ir are 16 bits. in order to visualize, 
	//	we need to convert them into 8 bits. The following fuctions are
	//	mean to do this.
	/**********************************************************************/
	
	/** \brief Convert a 16-bit depth map into a 8-bit image for visualization purpose
	 *	\param[in] pDepth The depth map (16-bit)
	 *	\param[out] pVisibleDepth The converted depth map (8-bit)
	 *	\param[in] size The number of pixels
	 *	\param[in] maxDepth The maximum possible number in the input depth map (for normalization purpose)
	 */
	template<class T>
	void VisibleDepth(const uInt16 *pDepth, T *pVisibleDepth, const int size, const double maxDepth = 5.0*1000)
	{
//		const double MAX_DEPTH = 10.0 * 1000;
		for (int i=0; i<size; i++)
		{
			pVisibleDepth[i] = static_cast<T>( (/*1 - */pDepth[i]/maxDepth) * 255.0);
		}
	}
	
	/** \brief Convert a 16-bit depth map into a 3-channel 8-bit rgb image for visualization purpose
	 *	\param[in] pDepth The depth map (16-bit)
	 *	\param[out] pVisibleDepth The converted depth map (3-channel, 8-bit)
	 *	\param[in] size The number of pixels
	 *	\param[in] maxDepth The maximum possible number in the input depth map (for normalization purpose)
	 */
	template<class T>
	void VisibleDepthRGB(const uInt16 *pDepth, T *pVisibleDepth, const int size, const double maxDepth = 5.0*1000)
	{
//		const double MAX_DEPTH = 10.0 * 1000;
		for (int i=0; i<size; i++)
		{
			const T &val = static_cast<T>( (/*1 - */pDepth[i]/maxDepth) * 255.0);
			pVisibleDepth[3*i] = pVisibleDepth[3*i+1] = pVisibleDepth[3*i+2] = val;
		}
	}


	
	/** \brief Convert a 16-bit IR map into a 8-bit image for visualization purpose
	 *	\param[in] pIr The input IR image (16-bit)
	 *	\param[out] pVisibleIr The converted IR image(8-bit)
	 *	\param[in] size The number of pixels	 
	 */
	template<class T>
	void VisibleIr(const uInt16 *pIr, T *pVisibleIr, const int size)
	{	
		for (int i=0; i<size; i++)
		{
			pVisibleIr[i] = static_cast<T>( pIr[i] >> 1);
		}
	}


};//namespace rm



#endif //CAMERA_H_