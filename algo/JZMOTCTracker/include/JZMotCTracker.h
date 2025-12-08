#pragma once
#include<opencv2/opencv.hpp>
#include <deque>
#include <vector>
#include <memory>
#include  <set>
#include "common.h"
//App BaseLine
typedef struct ObjectInfo{
	///track info  ���ڿɰ������Ӹ���
	int class_id;                                      //目标检测类别 class label id
	float class_score;
	int attribute_id;                                 //属性识别ID
	float attribute_score;
	bool is_center;
	cv::Rect bbox;                                      //the last update
	cv::Rect predbox;                              //kfÿһ��Ԥ���Ŀ��
	std::deque<cv::Point> trajectory;              //history trajectory points
	std::deque<BBoxInfo> m_hisInfos;      //Ŀ����ʷ����������Ϣ�������л���

	///Ӧ�ò���� ,���ڿɰ������Ӹ���
	unsigned int match_quality = 0;
	bool isUpload = false;

	///kalman tracker's init function
	void initKalmanTracker(cv::Rect &box);
	class ImplKalmanTracker;
	std::shared_ptr<ImplKalmanTracker> kf;
};


class  MotCTracker {
public:
	 MotCTracker();

	virtual  ~MotCTracker();

	int init(int class_num = 20, int queue_num = 100, int loss_num = 6, float iou_thresh = 0.4);

	//simple bbox track
	void track(std::vector<cv::Rect> &bboxes,std::vector<int> &class_ids);

	//all  ��������������Ϣ�ĸ���
	void track(std::vector<BBoxInfo> &in_objs);

	//����--Ĭ�ϲ����Ǿ�����֤�Ϻõ�
	void setIOUThresh(float iouThesh);

	void setLossNum(int loss_num);

	int getClassNum();

	std::vector<std::vector<ObjectInfo>> getHistoryObjects();

private:
	class MotImpl;
	MotImpl *motImpl=nullptr;
};