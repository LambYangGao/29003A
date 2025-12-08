Civilian_Vehicles:0
Military_Vehicle:1
Civilian_Ship:2
Military_Ship:3


void Classification::init(const cv::Mat originImage, cv::Mat &paddingImage)
{
    cv::cvtColor(originImage, originImage, cv::COLOR_BGR2RGB);
    int paddingImageH = paddingImageH_;
    int paddingImageW = paddingImageW_;
    int originImageH = originImage.rows;
    int originImageW = originImage.cols;
    float ratioH = static_cast<float>(paddingImageH) / originImageH;
    float ratioW = static_cast<float>(paddingImageW) / originImageW;

    float r = std::min(ratioH, ratioW);
    int resizedH = std::ceil(originImageH * r);
    int resizedW = std::ceil(originImageW * r);

    cv::Mat resizedImage;
    cv::resize(originImage, resizedImage, cv::Size(resizedW, resizedH));

    double dw = paddingImageW - resizedW;
    dw /= 2;
    double dh = paddingImageH - resizedH;
    dh /= 2;

    double top, bottom, left, right;
    top = round(dh - 0.1);
    bottom = round(dh + 0.1);
    left = round(dw - 0.1);
    right = round(dw + 0.1);
    cv::copyMakeBorder(resizedImage, paddingImage, top, bottom, left, right, cv::BORDER_CONSTANT, cv::Scalar(114, 114, 114));
}

