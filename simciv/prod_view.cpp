#include "world_ui.h"
#include "economy.h"

namespace simciv
{

USING_NS_CC;
using namespace std;
using namespace ui;


std::string factory_strings[4] = {
	"factory_red.png", "factory_blue.png", "factory_green.png", "factory_yellow.png"
};

std::string mine_strings[4] = {
	"mine_red.png", "mine_blue.png", "mine_green.png", "mine_yellow.png"
};

#define defvec(vec, ...) \
	static const string arr ## vec[] = { __VA_ARGS__ }; \
	vector<string> vec (arr ## vec, arr ## vec + sizeof(arr ## vec) / sizeof(arr ## vec[0]) );


ProdView* ProdView::create(WorldModel* model)
{
	ProdView* result = new ProdView();
	result->_model = model;
	if (result && result->init())
	{
		result->autorelease();
		return result;
	}
	else
	{
		CC_SAFE_DELETE(result);
		return nullptr;
	}
}

bool ProdView::init()
{
	if (!MapView::init())
	{
		return false;
	}

	auto visibleSize = Director::getInstance()->getVisibleSize();
	auto w = visibleSize.width;
	auto h = visibleSize.height;

	//auto listener = EventListenerTouchOneByOne::create();
	//listener->onTouchMoved = CC_CALLBACK_2(ProdView::onTouchMoved, this);
	//listener->onTouchEnded = CC_CALLBACK_2(ProdView::onTouchEnded, this);
 //   _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

	

	Vec2 topLeft = Vec2(visibleSize.width / 2.0f, visibleSize.height / 2.0f);

	// right menu
	auto right_menu = VBox::create();

	auto s = Size(20, 20);
	LinearLayoutParameter* p = LinearLayoutParameter::create();
	p->setGravity(LinearLayoutParameter::LinearGravity::TOP);
	p->setMargin(Margin(2, 2, 2, 2));
	LinearLayoutParameter* q = LinearLayoutParameter::create();
	q->setGravity(LinearLayoutParameter::LinearGravity::LEFT);
	//q->setMargin(Margin(2, 2, 2, 2));

	for (int i = 0; i < 1; ++i)
	{
		auto hbox = HBox::create();
		hbox->setLayoutParameter(q);

		auto fs = factory_strings[i];
		auto factory_button = Button::create(fs, fs, fs);
		factory_button->addTouchEventListener([this, i](Ref* w, Widget::TouchEventType e) {
			// _product_id = i;
			_mode = IT_FACTORY;
		});
		
		factory_button->ignoreContentAdaptWithSize(false);
		factory_button->setSize(s);
		factory_button->setLayoutParameter(p);
		hbox->addChild(factory_button);

		fs = mine_strings[i];
		auto mine_button = Button::create(fs, fs, fs);
		mine_button->addTouchEventListener([this, i](Ref* w, Widget::TouchEventType e) {
			// _product_id = i;
			_mode = IT_MINE;
		});
		mine_button->setSize(s);
		mine_button->setLayoutParameter(p);
		mine_button->ignoreContentAdaptWithSize(false);
		hbox->addChild(mine_button);
		hbox->setSize(Size(50, 25));
		right_menu->addChild(hbox);
	}

	this->addChild(right_menu);
	//right_menu->setAnchorPoint(Vec2(1, 1)); // doesn't have effect
	//right_menu->setPosition(Vec2(w - 13, h));
	right_menu->setPosition(Vec2(w - 50, h));

	// left menu
	// auto 
	left_menu = VBox::create();
	p = LinearLayoutParameter::create();
	p->setMargin(Margin(2, 2, 2, 2));
	p->setGravity(LinearLayoutParameter::LinearGravity::LEFT);

	int hh = 30;
	int marginy = 20;

	// ==============================================================================================
	// PRODUCT
	defvec(vec0, "_factory_red.png", "_factory_blue.png", "_factory_green.png", "_factory_yellow.png")
	auto rb = RadioBox::create(&_product_id, vec0, hh, marginy);
	left_menu->addChild(rb);

	// ==============================================================================================
	// PRICE - VOL - RES
	_show_price_vol_mode = 0;
	defvec(vec1, "Price", "Volume", "Res.")
	rb = RadioBox::create(&_show_price_vol_mode, vec1, hh, marginy);
	left_menu->addChild(rb);

	// ==============================================================================================
	// SUPPLY - CONSUMPTION
	_show_sup_con_mode = 2;
	defvec(vec2, "Supply", "Cons.", "Both")
	rb = RadioBox::create(&_show_sup_con_mode, vec2, hh, marginy);
	left_menu->addChild(rb);

	// ==============================================================================================
	// BACKGROUND
	auto cb_bck = labelled_cb("Background", false, [this](Ref* pSender,CheckBox::EventType type) {
		_map->setVisible(type == CheckBox::EventType::SELECTED);
	});
	cb_bck->setLayoutParameter(p);
	left_menu->addChild(cb_bck);

	// ==============================================================================================--
	// TRANSPORT
	_show_transport = true;
	auto cb_transport = labelled_cb("Routes", _show_transport, [this](Ref* pSender,CheckBox::EventType type) {
		_show_transport = !_show_transport;
	});
	cb_transport->setLayoutParameter(p);
	left_menu->addChild(cb_transport);

	left_menu->setAnchorPoint(Vec2(0, 1));
	left_menu->setPosition(Vec2(0, h));

	this->addChild(left_menu);

	//set_price_vol_mode(0);
	//set_sup_con_mode(2);
	_show_grid = false;

	
	//Node*
	_items = Node::create(); 
	_items->setAnchorPoint(Vec2(0, 0));
	_items->setPosition(Vec2(visibleSize / 2) - _table / 2);
	//_items->setPosition(Vec2(0, 0));
	
	this->addChild(_items, 0, 1);
	_items->setLocalZOrder(1);
	_items->setContentSize(_table);

	//auto cb = [](float f) {};
	// _draw_tiles.schedule(schedule_selector(WorldUI::tick), this, 0.1, kRepeatForever, 0, false);
	
	_mode = IT_FACTORY;
	_product_id = 0;

	add_item(IT_FACTORY, _table.width / 3, _table.height / 2);
	add_item(IT_MINE, 2 * _table.width / 3, _table.height / 2);

	return true;
}

bool is_map_point(cocos2d::Vec2& p)
{
	//return p.y < 557 || p.x > 366;
	return p.x > 370 || p.y > 222;
}

Item* ProdView::add_item(ItemType type, int x, int y)
{
	int ax = x / cs;
	int ay = y / cs;
	if (ax < 0 || ay < 0 || ax >= _model->width() || ay >= _model->height()) return NULL;

	Area* a = _model->get_area(ax, ay);

	switch (type)
	{
	case simciv::IT_MINE:
		{
			auto mine1 = Sprite::create(mine_strings[_product_id]);
			mine1->setPosition(x, y);
			mine1->setScale(0.05);
			_items->addChild(mine1);
			_model->add_prod(a, _product_id, 100, 10);
		}
		break;
	case simciv::IT_FACTORY:
		{
			auto factory1 = Sprite::create(factory_strings[_product_id]);
			factory1->setPosition(x, y);
			factory1->setScale(0.2);
			_items->addChild(factory1);
			_model->add_prod(a, _product_id, -100, 100);
		}
		break;
	default:
		break;
	}

	return NULL;
}

void ProdView::onTouchEnded(Touch* touch, Event  *event)
{
	auto s = touch->getStartLocation();
	auto p = touch->getLocation();
	if (is_map_point(touch->getLocationInView()) && (p - s).length() < 10)
	{
		p = _items->convertToNodeSpace(p);
		add_item(_mode, p.x, p.y);
	}
}


void ProdView::onTouchMoved(Touch* touch, Event  *event)
{
	if (is_map_point(touch->getLocationInView()))
	{
		auto diff = touch->getDelta();
		_map->setPosition(_map->getPosition() + diff);
		_items->setPosition(_items->getPosition() + diff);
	}
}

void ProdView::onDraw(const Mat4 &transform, uint32_t flags)
{
	// calculate roads
	_model->products()[_product_id]->routes_to_areas(_product_id);

    Director* director = Director::getInstance();
    director->pushMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
    director->loadMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW, transform);
    
    //draw
    CHECK_GL_ERROR_DEBUG();
    
    // draw a simple line
    // The default state is:
    // Line Width: 1
    // color: 255,255,255,255 (white, non-transparent)
    // Anti-Aliased
    //  glEnable(GL_LINE_SMOOTH);
    //DrawPrimitives::drawLine( VisibleRect::leftBottom(), VisibleRect::rightTop() );
	auto b = _map->getBoundingBox();
	//DrawPrimitives::drawLine( Vec2(b.getMinX(), b.getMinY()), Vec2(b.getMaxX(), b.getMaxY()));
	//DrawPrimitives::drawLine( Vec2(b.getMinX(), b.getMaxY()), Vec2(b.getMaxX(), b.getMinY()));

	if (_show_grid)
	{
		glLineWidth(1);
		float x = b.getMinX();
		for (int i = 0; i <= _model->width(); ++i, x += cs)
		{
			DrawPrimitives::drawLine( Vec2(x, b.getMinY()), Vec2(x, b.getMaxY()));
		}
		float y = b.getMinY();
		for (int i = 0; i <= _model->height(); ++i, y += cs)
		{
			DrawPrimitives::drawLine( Vec2(b.getMinX(), y), Vec2(b.getMaxX(), y));
		}
	}

	// draw_rect(5, 5, 1);

	double min_v = 1000;
	double max_v = 0;
	double min_vol = 1000;
	double max_vol = 0;

	if (_show_price_vol_mode == 0)
	{
		if (_show_sup_con_mode == 2)
		{
			for (Area* a: _model->areas())
			{
				auto& p = _model->get_prod(a, _product_id);
				double v = p.p;
				min_v = std::min(min_v, v);
				max_v = std::max(max_v, v);
				double vol = p.v_con + p.v_sup;
				min_vol = std::min(min_vol, vol);
				max_vol = std::max(max_vol, vol);
			}
			double d = max_v - min_v;
			double d_vol = max_vol - min_vol;

			for (Area* a: _model->areas())
			{
				auto& p = _model->get_prod(a, _product_id);
				double v = p.p;
				double r = d == 0 ? 0.5 : (v - min_v) / d;
				double vol = p.v_con + p.v_sup;
				draw_rect(a->x, a->y, r, vol / d_vol);
			}
		}
		else if (_show_sup_con_mode == 0)
		{
			for (Area* a: _model->areas())
			{
				auto& p = _model->get_prod(a, _product_id);
				double v = p.p_sup;
				min_v = std::min(min_v, v);
				max_v = std::max(max_v, v);
				double vol = p.v_sup;
				min_vol = std::min(min_vol, vol);
				max_vol = std::max(max_vol, vol);
			}
			double d = max_v - min_v;
			double d_vol = max_vol - min_vol;

			for (Area* a: _model->areas())
			{
				auto& p = _model->get_prod(a, _product_id);
				double v = p.p_sup;
				double r = d == 0 ? 0.5 : (v - min_v) / d;
				double vol = p.v_sup;
				draw_rect(a->x, a->y, r, vol / d_vol);
			}
		}
		else if (_show_sup_con_mode == 1)
		{
			for (Area* a: _model->areas())
			{
				auto& p = _model->get_prod(a, _product_id);
				double v = p.p_con;
				min_v = std::min(min_v, v);
				max_v = std::max(max_v, v);
				double vol = p.v_con;
				min_vol = std::min(min_vol, vol);
				max_vol = std::max(max_vol, vol);
			}
			double d = max_v - min_v;
			double d_vol = max_vol - min_vol;

			for (Area* a: _model->areas())
			{
				auto& p = _model->get_prod(a, _product_id);
				double v = p.p_con;
				double r = d == 0 ? 0.5 : (v - min_v) / d;
				double vol = p.v_con;
				draw_rect(a->x, a->y, r, vol / d_vol);
			}
		}
	}
	else if (_show_price_vol_mode == 2)
	{
		for (Area* a: _model->areas())
		{
			auto& p = _model->get_prod(a, _product_id);
			draw_rect_green(a->x, a->y, p.resource, 1);
		}
	}

	if (_show_transport)
	{
		//DrawPrimitives::setDrawColor4F(0, 0, 1, 1);
		glLineWidth(3);
		double scale = 0.1;

		for (Area* a: _model->areas())
		{
			double x, y;
			a->get_trans(_product_id, x, y);
			Rect r = get_rect(a->x, a->y);
			Vec2 p = Vec2(r.getMidX(), r.getMidY());
			DrawPrimitives::drawLine(p, Vec2(p.x + scale * x, p.y + scale * y));
		}
	}
}

void ProdView::set_price_vol_mode(int i)
{
	int k = 0;
	for (Widget* l: _cb_price_vol_mode)
	{
		((CheckBox*)l->getChildren().at(0))->setSelectedState(i == k++);
	}
	_show_price_vol_mode = i;
}

void ProdView::set_sup_con_mode(int i)
{
	int k = 0;
	for (Widget* l: _cb_sup_con_mode)
	{
		((CheckBox*)l->getChildren().at(0))->setSelectedState(i == k++);
	}
	_show_sup_con_mode = i;
}


}