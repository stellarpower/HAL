#include "SplitDriver.h"

#include <unistd.h>

namespace hal
{

SplitDriver::SplitDriver(std::shared_ptr<CameraDriverInterface> Input, std::vector<hal::ImageRoi>& vROIs, bool bCopy)
    : m_Input(Input), m_vROIs(vROIs), m_bCopy( bCopy )
{

}

bool SplitDriver::Capture( pb::CameraMsg& vImages )
{

    pb::CameraMsg pbIn;
    if( m_Input->Capture( pbIn ) == false ) {
        return false;
    }

    if( pbIn.image_size() > 1 ) {
        std::cerr << "error: Split is expecting 1 image but instead got " << pbIn.image_size() << "." << std::endl;
        return false;
    }

    const pb::ImageMsg& InImg = pbIn.image(0);

    for( unsigned int ii = 0; ii < m_vROIs.size(); ++ii ) {

        pb::ImageMsg* pImg = vImages.add_image();
//        pImg->CopyFrom( InImg );

        pImg->set_format( InImg.format() );
        pImg->set_type( InImg.type() );
        pImg->set_timestamp( InImg.timestamp() );

        const ImageRoi& ROI = m_vROIs[ii];
        const unsigned int nTotalBytes = ROI.w * ROI.h;

        pImg->set_width( ROI.w );
        pImg->set_height( ROI.h );
        pImg->mutable_data()->resize( nTotalBytes );

        unsigned char* pS = (unsigned char*)&InImg.data().front();
        unsigned char* pD = (unsigned char*)&pImg->mutable_data()->front();
        for( unsigned int ii = 0; ii < ROI.h; ++ii ) {
            memcpy( pD, pS + (InImg.width()*(ii+ROI.y)) + ROI.x, ROI.w );
            pD = pD + ROI.w;
        }
    }

//    sleep(1);

    return true;
}

std::string SplitDriver::GetDeviceProperty(const std::string& sProperty)
{
    return m_Input->GetDeviceProperty(sProperty);
}

}