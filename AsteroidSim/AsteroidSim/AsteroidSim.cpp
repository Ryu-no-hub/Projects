// SFML_party.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include "pch.h"
#include <iostream>
#include <SFML/Graphics.hpp>
#include <time.h>
#include <list>
#include <algorithm>
#define _USE_MATH_DEFINES
#include <math.h>
#include <thread>
#include <iterator>

using namespace sf;

bool collisions = true, ship = false;
const int W = 1600;
const int H = 1000;
const float power = pow(10, -28);
const double G = 6.67408f * power;
const double V_m = 4;
int N = 14, speed = 100, th_num = N / 20;
double DEGTORAD = 0.017453f;

class Animation
{
public:
	double Frame, speed;
	Sprite sprite;
	std::vector<IntRect> frames;

	Animation() {};
	Animation(Texture &t, int x, int y, int w, int h, int count, double Speed) {
		Frame = 0;
		speed = Speed;
		//std::cout << x << " " << y << " " << w << " " << h << " " << count << " " << Speed << std::endl;
		for (int i = 0; i < count; i++)
			frames.push_back(IntRect(x + i * w, y, w, h));
		sprite.setTexture(t);
		sprite.setOrigin(w / 2, h / 2);
		sprite.setTextureRect(frames[0]);
	}

	void update()
	{
		Frame += speed;
		int n = frames.size();
		if (Frame >= n) Frame -= n;
		if (n > 0) sprite.setTextureRect(frames[int(Frame)]);
	}

	bool isEnd()
	{
		return Frame + speed >= frames.size();
	}
};

class Entity
{
public:
	double x, y, dx, dy, R, angle, Volume;
	bool life;
	double mass;
	std::string name;
	Animation anim;
	Entity() { life = 1; };

	void settings(Animation &a, int X, int Y, double m, float dX, float dY, float volume, double Angle = 0)
	{
		x = X; y = Y;
		dx = dX; dy = dY;
		angle = Angle;
		Volume = volume;
		R = pow(0.75 / M_PI * volume, 1.0 / 3); mass = m; anim = a;
		//std::cout <<"Settings "<< x << " " << y << " " <<R << " "<< name<<std::endl;
	}
	virtual void update(std::list<Entity*> &entity/*, RenderWindow &app*/) {};
	void draw(RenderWindow &app)
	{
		anim.sprite.setPosition(x, y);
		anim.sprite.setRotation(angle + 90);
		app.draw(anim.sprite);

		/*CircleShape circle(R);
		circle.setFillColor(Color(255, 0, 0, 100));
		circle.setPosition(x, y);
		circle.setOrigin(R, R);
		app.draw(circle);*/
	};
	virtual ~Entity() { /*std::cout << "distructed (" << this->x<<", "<<this->y<<") "<<this->name<< std::endl;*/ };
};

class Body :public Entity
{
public:
	Body()
	{
		name = "Body";
	}
	void update(std::list<Entity*> &entity/*, RenderWindow &app*/) override
	{/*
		Mutex m;
		m.lock();*/
		for (auto i : entity)
		{
			if (x != i->x && y != i->y)
			{
				double distance = sqrt((x - i->x)*(x - i->x) + (y - i->y)*(y - i->y));
				double a = ((-G) * i->mass) / (distance * distance);
				double dirangle;
				dirangle = acos((y - i->y) / distance);
				if (x - i->x <= 0)
					dirangle += M_PI;
				dx += sin(dirangle)*a;
				if (x - i->x > 0)
					dy += cos(dirangle)*a;
				else dy -= cos(dirangle)*a;
			}
		}
		x += dx;
		y += dy;
		//std::cout << "update (" << x << ", " << y << ")" << std::endl;
		/*CircleShape circle(R);
		circle.setFillColor(Color(255, 255, 0, 100));
		circle.setPosition(x, y);
		circle.setOrigin(R, R);
		app.draw(circle);*/
		if (x > W || x<0 || y>H || y < 0) life = 0;
		/*m.unlock();*/
	}
};


class Player :public Entity
{
public:
	bool thrust;
	Player()
	{
		name = "player";
	}
	void update(std::list<Entity*> &entity/*, RenderWindow &app*/) override
	{
		for (auto i : entity)
		{
			if (x != i->x && y != i->y)
			{
				double distance = sqrt((x - i->x)*(x - i->x) + (y - i->y)*(y - i->y));
				double a = ((-G) * i->mass) / (distance * distance);
				double dirangle;
				dirangle = acos((y - i->y) / distance);
				if (x - i->x <= 0)
					dirangle += M_PI;
				dx += sin(dirangle)*a;
				if (x - i->x > 0)
					dy += cos(dirangle)*a;
				else dy -= cos(dirangle)*a;
			}
		}
		if (thrust)
		{
			//std::cout << "Thrusting! " << dx << " " << dy << std::endl;
			dx += cos(angle*DEGTORAD)*0.1;
			dy += sin(angle*DEGTORAD)*0.1;
		}
		int maxspeed = 15;
		float speed = sqrt(dx*dx + dy * dy);
		if (speed > maxspeed)
		{
			dx *= maxspeed / speed;
			dy *= maxspeed / speed;
		}
		x += dx;
		y += dy;
		if (x > W) x = 0; if (x < 0) x = W;
		if (y > H) y = 0; if (y < 0) y = H;
	}
};

bool isCollide(Entity *a, Entity *b)
{
	return (b->x - a->x)*(b->x - a->x) + (b->y - a->y)*(b->y - a->y) < (a->R + b->R)*(a->R + b->R);
}

void rotate_V(double speed, double x, double y, double& V0x, double& V0y)
{
	x -= W / 2; y -= H / 2;
	float Speed = speed / sqrt(x*x + y * y);

	if (y > 0)
		V0x = -cos(x / (W / +2) * M_PI / 2)*Speed;
	else
		V0x = cos(x / (W / 2)  * M_PI / 2)*Speed;

	if (x > 0)
		V0y = sqrt(Speed*Speed - V0x * V0x);
	else
		V0y = -sqrt(Speed*Speed - V0x * V0x);
	//std::cout << "(" << x << ", " << y << ")  dx = " << V0x<<", dy = " << V0y<<",   speed = "<<speed<<std::endl;
}
double R_from_V(double V)
{
	return pow(0.75 / M_PI * V, 1.0 / 3);
}
void th_update(std::list<Entity*> &entities/*, RenderWindow &window*/)
{
	for (auto i = entities.begin(); i != entities.end();)
	{
		Entity *e = *i;
		e->update(entities/*, window*/);
		e->anim.update();
		if (e->life == false) { i = entities.erase(i); delete e; }
		else i++;
	}
}

int main()
{
	double Msun = 2 * pow(10, 30);
	double Me = 5.97*pow(10, 26);
	double Mmun = 7.36 * pow(10, 22);
	double Ma = 9 * pow(10, 19);
	int a_Vol = 10000;

	RenderWindow window(VideoMode(W, H), "System");
	window.setFramerateLimit(60);
	srand(time(0));
	Texture t1, t2, t3, t4, t5, t6, t7;
	t1.loadFromFile("images/sun_2.png");
	t2.loadFromFile("images/Earth.png");
	t3.loadFromFile("images/background.jpg");
	t4.loadFromFile("images/sailor-moon.png");
	t5.loadFromFile("images/explosions/type_C.png");
	t6.loadFromFile("images/asteroid.png");
	t7.loadFromFile("images/spaceship.png");

	Sprite background;
	background.setTexture(t3);

	Animation explosion(t5, 0, 0, 256, 256, 48, 0.5);
	Animation earth(t2, 0, 0, 42, 42, 1, 0);
	Animation sun(t1, 0, 0, 894, 894, 1, 0);
	Animation moon(t4, 0, 0, 45, 46, 1, 0);
	Animation asteroid(t6, 0, 0, 850, 837, 1, 0);
	Animation player(t7, 40, 0, 40, 40, 1, 0);
	Animation player_go(t7, 40, 40, 40, 40, 1, 0);
	Animation sBullet(t7, 40, 40, 40, 40, 1, 0);

	std::list<Entity*> entities, th_entities;
	std::list<std::thread> threads;
	std::list<Entity*>::iterator it, advanced_it;

	sun.sprite.scale(0.15, 0.15);
	earth.sprite.scale(0.5, 0.5);
	moon.sprite.scale(0.5, 0.5);
	asteroid.sprite.scale(0.01, 0.01);

	Body *a = new Body();
	a->settings(sun, W / 2, H / 2, Msun, 0, 0, 500000);
	entities.push_back(a);

	//Body *b = new Body();
	//b->settings(moon, 360, 190, Mmun, 1, -0.8, 10);
	//entities.push_back(b);


	Player *p = new Player();
	p->settings(player, W / 4, H / 4, pow(10, 10), 0, 0, 10000, 0);
	entities.push_back(p);

	double V0x = 0, V0y = 0;

	for (int j = 1; j < N; j++)
		for (int i = 1; i < N; i++)
		{
			Body *b = new Body();
			b->name = "asteroid";
			rotate_V(speed, W / N * i, H / N * j, V0x, V0y);
			b->settings(asteroid, W / N * i, H / N * j, Ma, V0x*V_m, V0y*V_m, a_Vol / (N - 1));
			entities.push_back(b);
		}
	while (window.isOpen())
	{
		advanced_it = it = entities.begin();
		Event event;
		while (window.pollEvent(event));
		{
			if (event.type == Event::Closed)
				window.close();
		}
		if (Keyboard::isKeyPressed(Keyboard::Right)) p->angle += 3;
		if (Keyboard::isKeyPressed(Keyboard::Left))  p->angle -= 3;
		if (Keyboard::isKeyPressed(Keyboard::Up)) p->thrust = true;
		else p->thrust = false;

		for (auto i = entities.begin(); i != entities.end();)
		{
			Entity *e = *i;
			e->update(entities);
			e->anim.update();
			if (e->life == false) { i = entities.erase(i); delete e; }
			else i++;
		}

		//for (int k = 0; k < entities.size(); k += 160000/pow(num, 2)) 
		//{
		//	th_entities.clear();
		//	//std::cout << (*advanced_it)->x << " " << (*advanced_it)->y << " " << std::endl;
		//	if (k + 160000 / pow(num, 2) <= entities.size() - 1)
		//		std::advance(advanced_it, 160000 / pow(num, 2));
		//	else
		//		std::advance(advanced_it, std::distance(it, entities.end()));
		//	copy(it, advanced_it, back_inserter(th_entities));
		//	std::cout<< th_entities.size()<<" "<< std::endl;
		//	threads.push_back(std::thread(th_update, std::ref(th_entities)/*, std::ref(window)*/));
		//	it = advanced_it;
		//}
		//for (auto& th : threads)
		//{
		//	th.join();
		//}
		//threads.clear();

		window.clear();
		for (auto a : entities)
		{
			for (auto b : entities)
				if (isCollide(a, b) && ((a != b && collisions && a->name != "explosion" && b->name != "explosion" && a->name != "player" && b->name != "player") || (a->name == "player" && b->mass >= Msun)))
				{
					if (a->life == true && b->life == true)
					{
						Entity *p = new Entity();
						p->settings(explosion, (a->x + b->x) / 2, (a->y + b->y) / 2, 0, 0, 0, 4);
						p->name = "explosion";
						entities.push_back(p);

						std::cout << "Collision (" << a->x << ", " << a->y << ") - (" << b->x << ", " << b->y << " " << a->name << " - " << b->name << std::endl;

						Entity *big = a->R > b->R ? a : b;
						if (big == b) { b = a; a = big; }
						//std::cout << big->name <<" "<< a->R << " " << b->R<<" " << a->R + (b->R) / 2 << std::endl;
						double vdx = (a->mass * a->dx + b->mass * b->dx) / (a->mass + b->mass);
						double vdy = (a->mass * a->dy + b->mass * b->dy) / (a->mass + b->mass);
						big->anim.sprite.scale(R_from_V(a->Volume + b->Volume) / R_from_V(a->Volume), R_from_V(a->Volume + b->Volume) / R_from_V(a->Volume));
						a->settings(big->anim, big->x, big->y, a->mass + b->mass, vdx, vdy, a->Volume + b->Volume);
						b->life = false;
					}
				}
		}
		for (auto e : entities)
			if (e->name == "explosion")
				if (e->anim.isEnd()) e->life = 0;

		window.draw(background);

		for (auto i : entities)
			i->draw(window);
		window.display();
	}
	return 0;
}

//#include "pch.h"
//#include <SFML/Graphics.hpp>
//#include <time.h>
//#include <list>
//using namespace sf;
//
//const int W = 1200;
//const int H = 800;
//
//float DEGTORAD = 0.017453f;
//
//class Animation
//{
//public:
//	float Frame, speed;
//	Sprite sprite;
//    std::vector<IntRect> frames;
//
//	Animation(){}
//
//    Animation (Texture &t, int x, int y, int w, int h, int count, float Speed)
//	{
//	    Frame = 0;
//        speed = Speed;
//
//		for (int i=0;i<count;i++)
//         frames.push_back( IntRect(x+i*w, y, w, h)  );
//
//		sprite.setTexture(t);
//		sprite.setOrigin(w/2,h/2);
//        sprite.setTextureRect(frames[0]);
//	}
//
//
//	void update()
//	{
//    	Frame += speed;
//		int n = frames.size();
//		if (Frame >= n) Frame -= n;
//		if (n>0) sprite.setTextureRect( frames[int(Frame)] );
//	}
//
//	bool isEnd()
//	{
//	  return Frame+speed>=frames.size();
//	}
//
//};
//
//
//class Entity
//{
//public:
//float x,y,dx,dy,R,angle;
//bool life;
//std::string name;
//Animation anim;
//
//Entity()
//{
//  life=1;
//}
//
//void settings(Animation &a,int X,int Y,float Angle=0,int radius=1)
//{
//  anim = a;
//  x=X; y=Y;
//  angle = Angle;
//  R = radius;
//}
//
//virtual void update(){};
//
//void draw(RenderWindow &app)
//{
//  anim.sprite.setPosition(x,y);
//  anim.sprite.setRotation(angle+90);
//  app.draw(anim.sprite);
//
//  CircleShape circle(R);
//  circle.setFillColor(Color(255,0,0,170));
//  circle.setPosition(x,y);
//  circle.setOrigin(R,R);
//  //app.draw(circle);
//}
//
//virtual ~Entity(){};
//};
//
//
//class asteroid: public Entity
//{
//public:
//  asteroid()
//  {
//    dx=rand()%8-4;
//    dy=rand()%8-4;
//    name="asteroid";
//  }
//
//void  update()
//  {
//   x+=dx;
//   y+=dy;
//
//   if (x>W) x=0;  if (x<0) x=W;
//   if (y>H) y=0;  if (y<0) y=H;
//  }
//
//};
//
//
//
//class bullet: public Entity
//{
//public:
//  bullet()
//  {
//    name="bullet";
//  }
//
//void  update()
//  {
//   dx=cos(angle*DEGTORAD)*6;
//   dy=sin(angle*DEGTORAD)*6;
//  // angle+=rand()%6-3;
//   x+=dx;
//   y+=dy;
//
//   if (x>W || x<0 || y>H || y<0) life=0;
//  }
//
//};
//
//
//class player: public Entity
//{
//public:
//   bool thrust;
//
//   player()
//   {
//     name="player";
//   }
//
//   void update()
//   {
//     if (thrust)
//      { dx+=cos(angle*DEGTORAD)*0.2;
//        dy+=sin(angle*DEGTORAD)*0.2; }
//     else
//      { dx*=0.99;
//        dy*=0.99; }
//
//    int maxSpeed=15;
//    float speed = sqrt(dx*dx+dy*dy);
//    if (speed>maxSpeed)
//     { dx *= maxSpeed/speed;
//       dy *= maxSpeed/speed; }
//
//    x+=dx;
//    y+=dy;
//
//    if (x>W) x=0; if (x<0) x=W;
//    if (y>H) y=0; if (y<0) y=H;
//   }
//
//};
//
//
//bool isCollide(Entity *a,Entity *b)
//{
//  return (b->x - a->x)*(b->x - a->x)+
//         (b->y - a->y)*(b->y - a->y)<
//         (a->R + b->R)*(a->R + b->R);
//}
//
//
//int main()
//{
//    srand(time(0));
//
//    RenderWindow app(VideoMode(W, H), "Asteroids!");
//    app.setFramerateLimit(60);
//
//    Texture t1,t2,t3,t4,t5,t6,t7;
//    t1.loadFromFile("images/spaceship.png");
//    t2.loadFromFile("images/background.jpg");
//    t3.loadFromFile("images/explosions/type_C.png");
//    t4.loadFromFile("images/rock.png");
//    t5.loadFromFile("images/fire_blue.png");
//    t6.loadFromFile("images/rock_small.png");
//    t7.loadFromFile("images/explosions/type_B.png");
//
//    t1.setSmooth(true);
//    t2.setSmooth(true);
//
//    Sprite background(t2);
//
//    Animation sExplosion(t3, 0,0,256,256, 48, 0.5);
//    Animation sRock(t4, 0,0,64,64, 16, 0.2);
//    Animation sRock_small(t6, 0,0,64,64, 16, 0.2);
//    Animation sBullet(t5, 0,0,32,64, 16, 0.8);
//    Animation sPlayer(t1, 40,0,40,40, 1, 0);
//    Animation sPlayer_go(t1, 40,40,40,40, 1, 0);
//    Animation sExplosion_ship(t7, 0,0,192,192, 64, 0.5);
//
//
//    std::list<Entity*> entities;
//
//    for(int i=0;i<15;i++)
//    {
//      asteroid *a = new asteroid();
//      a->settings(sRock, rand()%W, rand()%H, rand()%360, 25);
//      entities.push_back(a);
//    }
//
//    player *p = new player();
//    p->settings(sPlayer,200,200,0,20);
//    entities.push_back(p);
//
//    /////main loop/////
//    while (app.isOpen())
//    {
//        Event event;
//        while (app.pollEvent(event))
//        {
//            if (event.type == Event::Closed)
//                app.close();
//
//            if (event.type == Event::KeyPressed)
//             if (event.key.code == Keyboard::Space)
//              {
//                bullet *b = new bullet();
//                b->settings(sBullet,p->x,p->y,p->angle,10);
//                entities.push_back(b);
//              }
//        }
//
//    if (Keyboard::isKeyPressed(Keyboard::Right)) p->angle+=3;
//    if (Keyboard::isKeyPressed(Keyboard::Left))  p->angle-=3;
//    if (Keyboard::isKeyPressed(Keyboard::Up)) p->thrust=true;
//    else p->thrust=false;
//
//
//
//    for(auto a:entities)
//     for(auto b:entities)
//    {
//      if (a->name=="asteroid" && b->name=="bullet")
//       if ( isCollide(a,b) )
//           {
//            a->life=false;
//            b->life=false;
//
//            Entity *e = new Entity();
//            e->settings(sExplosion,a->x,a->y);
//            e->name="explosion";
//            entities.push_back(e);
//
//
//            for(int i=0;i<2;i++)
//            {
//             if (a->R==15) continue;
//             Entity *e = new asteroid();
//             e->settings(sRock_small,a->x,a->y,rand()%360,15);
//             entities.push_back(e);
//            }
//
//           }
//
//      if (a->name=="player" && b->name=="asteroid")
//       if ( isCollide(a,b) )
//           {
//            b->life=false;
//
//            Entity *e = new Entity();
//            e->settings(sExplosion_ship,a->x,a->y);
//            e->name="explosion";
//            entities.push_back(e);
//
//            p->settings(sPlayer,W/2,H/2,0,20);
//            p->dx=0; p->dy=0;
//           }
//    }
//
//
//    if (p->thrust)  p->anim = sPlayer_go;
//    else   p->anim = sPlayer;
//
//
//    for(auto e:entities)
//     if (e->name=="explosion")
//      if (e->anim.isEnd()) e->life=0;
//
//    if (rand()%150==0)
//     {
//       asteroid *a = new asteroid();
//       a->settings(sRock, 0,rand()%H, rand()%360, 25);
//       entities.push_back(a);
//     }
//
//    for(auto i=entities.begin();i!=entities.end();)
//    {
//      Entity *e = *i;
//
//      e->update();
//      e->anim.update();
//
//      if (e->life==false) {i=entities.erase(i); delete e;}
//      else i++;
//    }
//
//
//
//   //////draw//////
//   app.draw(background);
//
//   for(auto i:entities)
//     i->draw(app);
//
//   app.display();
//    }
//
//    return 0;
//}
