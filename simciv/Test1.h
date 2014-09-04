#pragma once

#include "cocos2d.h"

USING_NS_CC;

class Test1 : public cocos2d::Layer
{
public:
    // Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
    virtual bool init();  

    // there's no 'id' in cpp, so we recommend returning the class instance pointer
    static cocos2d::Scene* scene();
    
    // a selector callback
    void menuCloseCallback(Ref* sender);
    
	void onTouchesMoved(const std::vector<Touch*>& touches, Event  *event);
	virtual void onEnter() override;
    // implement the "static node()" method manually
    CREATE_FUNC(Test1);
};

class Node
{

};