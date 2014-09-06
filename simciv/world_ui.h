#pragma once

#include "cocos2d.h"
#include "world_model.h"

namespace simciv
{

USING_NS_CC;

enum ItemType;
class Item;

class WorldUI : public cocos2d::Layer
{
public:
    // Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
    virtual bool init();  
	void init_menu();  

    // there's no 'id' in cpp, so we recommend returning the class instance pointer
    static cocos2d::Scene* scene();
    
    // a selector callback
    void menuCloseCallback(Ref* sender);
    
	void onTouchMoved(Touch* touch, Event  *event);
	bool onTouchBegan(Touch* touch, Event  *event);
	void onTouchEnded(Touch* touch, Event  *event);
	virtual void draw(Renderer *renderer, const Mat4 &transform, uint32_t flags) override;
	virtual void onEnter() override;
    // implement the "static node()" method manually
    CREATE_FUNC(WorldUI);

	Item* add_item(ItemType type, int x, int y);
protected:
	static const int cs = 100; // cell size
	cocos2d::Node* _items;
	cocos2d::Node* _map;
	WorldModel _model;
	ItemType _mode;
	CustomCommand _customCommand;
    void onDraw(const Mat4 &transform, uint32_t flags);
	void draw_rect(int x, int y, double rate);
    Rect get_rect(int x, int y);
	void tick(float f);
	Size _table;
	bool _show_grid;
	bool _show_price;
	Scheduler _draw_tiles;
};

enum ItemType
{
	IT_MINE,
	IT_FACTORY
};

class Item
{
public:
	Item(ItemType type, int x, int y);
protected:
	Sprite* _sprite;
};

}