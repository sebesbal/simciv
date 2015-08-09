#pragma once

#include "cocos2d.h"
#include "world_model.h"
#include "AppMacros.h"
#include "VisibleRect.h"
#include "ui\UICheckBox.h"
#include "ui\UILayout.h"
#include "ui\UIHBox.h"
#include "ui\UIVBox.h"
#include "ui\UIText.h"
#include "ui\UIButton.h"

#include "base\ccTypes.h"
namespace simciv
{

USING_NS_CC;

enum ItemType;
class Item;

#define defvec(vec, ...) \
	static const string arr ## vec[] = { __VA_ARGS__ }; \
	vector<string> vec (arr ## vec, arr ## vec + sizeof(arr ## vec) / sizeof(arr ## vec[0]) );

/// draw tiles, map background, routes
class MapView : public cocos2d::Layer
{
public:
	static MapView* create(WorldModel* model);
	virtual bool init() override;
	virtual void draw(Renderer *renderer, const Mat4 &transform, uint32_t flags) override;
	CustomCommand _customCommand;
    virtual void onDraw(const Mat4 &transform, uint32_t flags);
	virtual bool onTouchBegan(Touch* touch, Event  *event);
	virtual void onTouchEnded(Touch* touch, Event  *event);
	virtual void onTouchMoved(Touch* touch, Event  *event);

	void draw_rect(int x, int y, double rate, double alpha);
	void draw_rect_green(int x, int y, double rate, double alpha);
	void draw_vec(Vec2 a, Vec2 v);
    Rect get_rect(int x, int y);

protected:
	static const int cs = 50; // cell size
	Size _table;
	cocos2d::Node* _map;
	WorldModel* _model;
};

/// Draw mines and factories
class ProdView : public MapView
{
public:
	static ProdView* create(WorldModel* model);
	virtual bool init() override;
	void onTouchEnded(Touch* touch, Event  *event) override;
	void onTouchMoved(Touch* touch, Event  *event) override;
protected:
	ui::VBox* left_menu;
	int _product_id;
	int _show_price_vol_mode;
	int _show_sup_con_mode;
	bool _show_grid;
	bool _show_transport;
	ItemType _mode;
	cocos2d::Node* _items;
	Item* add_item(ItemType type, int x, int y);

	virtual void onDraw(const Mat4 &transform, uint32_t flags) override;
	
	std::vector<ui::Widget*> _cb_price_vol_mode;
	std::vector<ui::Widget*> _cb_sup_con_mode;

	void set_price_vol_mode(int i);
	void set_sup_con_mode(int i);
};


class AnimalView : public MapView
{
public:
	static AnimalView* create(WorldModel* model);
	virtual bool init() override;
	void onTouchEnded(Touch* touch, Event  *event) override;
	void onTouchMoved(Touch* touch, Event  *event) override;
protected:
	bool is_map_point(cocos2d::Vec2& p);
	virtual void onDraw(const Mat4 &transform, uint32_t flags) override;
	ui::VBox* left_menu;
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


ui::Layout* labelled_cb(std::string text, bool checked, ui::CheckBox::ccCheckBoxCallback cb);

class RadioBox: public ui::HBox
{
public:
	RadioBox (int* data, std::vector<std::string> labels, int hh, int marginy);
	void setSelected(int i);
	static RadioBox* create(int* data, std::vector<std::string> labels, int hh, int marginy);
	ui::Layout* labelled_radio(std::string text, ui::CheckBox::ccCheckBoxCallback cb);
	ui::Layout* image_radio(std::string img, ui::CheckBox::ccCheckBoxCallback cb);
	int hh;
	int marginy;
	std::vector<ui::Widget*> items;
	int* data;
};

/// The main ui
class WorldUI : public cocos2d::Layer
{
public:
    // Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
    virtual bool init() override;

    // there's no 'id' in cpp, so we recommend returning the class instance pointer
    static cocos2d::Scene* scene();
    
    // a selector callback
    void menuCloseCallback(Ref* sender);
	virtual void onEnter() override;
    // implement the "static node()" method manually
    CREATE_FUNC(WorldUI);
protected:
	int view_mode, new_view_mode;
	std::vector<Node*> views;
	WorldModel _model;
	void tick(float f);
};

}