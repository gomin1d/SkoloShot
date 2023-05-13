#pragma once
#include "anim.h"
#include "level.h"

Vector2i pixelPos; //Вектор мыши
Vector2f pos; //Вектор мыши
const int W = 32 * 31;
const int H = 32 * 17;

////////////////// ENTITY //////////////////

class Entity
{
public:
	AnimationManager anim; //Анимация персонажа
	std::vector<Object> obj; //Обьекты для проверки на столкновения
	float x, y, w, h, xSpeed, ySpeed; //Координары, ширина, высота, скорость
	float heal;
	bool isHeal;

	Entity(AnimationManager &A, int X, int Y) //Присваиваем значения
	{
		x = X;		
		y = Y;
		anim = A;
		isHeal = true;
		heal = 100;
	}

	void draw(RenderWindow &window)
	{
		anim.draw(window, x, y + h); //Вывод на экран
	}

	FloatRect getRect() //Возращаем коорды
	{
		return FloatRect(x, y, w, h); 
	}
};

////////////////// Player //////////////////

class Player : public Entity
{
private:
	bool left, right, up, down; //Для проверки нажатия клавиш
public:
	int score;
	bool isAttack;
	Text hpMonitor; //Текст
	Player(AnimationManager &A, Level &lev, Text &text, float time, int X, int Y) : Entity(A, X, Y) //Констрктор
	{
		obj = lev.GetAllObjects();
		w = anim.getW();
		h = anim.getH();
		ySpeed = xSpeed = 0;
		left = right = up = down = false;

		anim.set("right");
		anim.play();
		anim.tick(time);
		anim.pause();
		
		hpMonitor = text;
		hpMonitor.setColor(Color::Red);

		score = 0;
	}

	void attack()
	{
		if (isAttack)
		{
			heal -= 0.1;
			ySpeed = -0.1;
			xSpeed = (rand() % 5 - 2) / 10;
			isAttack = false;
		}

	}

	void update(float time) //Главная процедура обновления
	{
		KeyCheck();	//Какие клавиши нажаты?
		
		if (right) //Если нажато право
		{
			xSpeed = 0.1;
			anim.set("right");
			anim.play();
		}
		else if (left) //Если нажато влево
		{
			xSpeed = -0.1;
			anim.set("left");
			anim.play();
		}
		else //Если не нажато
		{
			xSpeed = 0;
			anim.pause();
		}

		if (up && ySpeed == 0) //Если нажато вверх и игрок не в прыжке
			ySpeed = -0.4;

		if (right || left) //Проверяем на столкновения по X, если мы в движении
		{
			if (Keyboard::isKeyPressed(Keyboard::LShift)) //Ускоряем, если жмем Left Shift
				xSpeed *= 2;

			x += xSpeed * time; //Движемся по X
			for (int i = 0; i < obj.size(); i++)
				if (getRect().intersects(obj[i].rect))
					if (obj[i].name == "solid") //Если столкнулись с объектами
					{
						if (xSpeed > 0)
						{
							x = obj[i].rect.left - w;
						}
						else if (xSpeed < 0)
						{
							x = obj[i].rect.left + obj[i].rect.width;
						}
						break;
					}
		}

		y += ySpeed * time; //Движемся по Y
		ySpeed += 0.0005*time; //Падаем

		for (int i = 0; i<obj.size(); i++)
			if (getRect().intersects(obj[i].rect))
			{
				if (obj[i].name == "solid") //Если столкнулись с объектами
				{
					if (ySpeed > 0)
					{
						y = obj[i].rect.top - h;
						ySpeed = 0;
					}
					else if (ySpeed < 0)
					{
						y = obj[i].rect.top + obj[i].rect.height;
						ySpeed = 0.001;

					}
					break;
				}
				else if (obj[i].name == "lestnica")
				{
					if (up)
						ySpeed = -0.1;
					if (down)
						ySpeed = 0.1;
					else if (FloatRect(x, y + h - 1, w, 1).intersects(FloatRect(obj[i].rect.left, obj[i].rect.top, obj[i].rect.width, 1)))
					{
						y = obj[i].rect.top - h;
						ySpeed = 0;
					}
				}
			}

		hpMonitor.setString("HP: " + std::to_string((int)heal) + "% Убийств: " + std::to_string(score)); //Вывод жизней
		anim.tick(time); //Анимация
	}
private:
	void KeyCheck() //проверяем клавиши
	{
		if (Keyboard::isKeyPressed(Keyboard::D)
			|| Keyboard::isKeyPressed(Keyboard::Right))
			right = true;
		else
			right = false;

		if (Keyboard::isKeyPressed(Keyboard::A)
			|| Keyboard::isKeyPressed(Keyboard::Left))
			left = true;
		else
			left = false;

		if (Keyboard::isKeyPressed(Keyboard::W)
			|| Keyboard::isKeyPressed(Keyboard::Up))
			up = true;
		else
			up = false;

		if (Keyboard::isKeyPressed(Keyboard::S))
			down = true;
		else
			down = false;
	}
};

////////////////// Gun //////////////////

class Gun
{
private:
	String name;
	float gunX;
	float gunY;
	float rotation;
public:
	Sprite gun;
	Gun(Texture &t, String n)
	{
		gun.setTexture(t);
		gun.setTextureRect(IntRect(95, 101, 73, 26));

		name = n;
	}

	void set(String n, int x, int y, int w, int h)
	{
		gun.setTextureRect(IntRect(x, y, w, h));
		name = n;
	}

	void update(float time, RenderWindow &window, Player &p)
	{
		pixelPos = Mouse::getPosition(window);		//забираем коорд курсора			  ////
		pos = window.mapPixelToCoords(pixelPos);	//переводим их в игровые				//
		gunX = pos.x - p.x - 15;					//Вектор х								//
		gunY = pos.y - p.y - 35;					//Вектор y								//
		rotation = (atan2(gunX, gunY)) * 180 / 3.14159265;//В градусы						//
																							//
		if (rotation <= 180 && rotation >= 0)	//Поворачиваем оружие						//
		{																					// Поворачиваем оружие
			gun.setTextureRect(IntRect(95, 101, 73, 26));									//
			gun.setOrigin(20, 0);															//
			gun.setRotation(85 - rotation);	//поворачиваем спрайт							//
		}																					//
		else if (rotation <= 0 && rotation >= -180)	//Поворачиваем оружие					//
		{																					//
			gun.setTextureRect(IntRect(168, 101, -73, 26));									//
			gun.setOrigin(53, 0);															//
			gun.setRotation(275 - rotation);	//поворачиваем спрайт						//
		}																				  ////

		gun.setPosition(p.x + 15, p.y + 35);
	}

};

////////////////// Monster //////////////////

class Monster : public Entity
{
private:
	int timer, r;
public:
	int heal;
	Text text;
	bool isLife;

	Monster(AnimationManager &A, Level &lev, float time, int X, int Y) : Entity(A, X, Y)
	{
		obj = lev.GetObjects("solid");
		w = anim.getW();
		h = anim.getH();
		xSpeed = 0.1;

		heal = 100;
		isLife = true;
		timer = r = 0;

		anim.set("left");
		anim.setLoop(true);
		anim.play();
		anim.set("right");
		anim.setLoop(true);
		anim.play();
		anim.tick(time);
	}

	void attack()
	{
		heal -= 10 + rand() % 20;
		if (heal < 1)
			isLife = false;
	}

	void update(float time)
	{
		x += xSpeed * time; //Идем
		for (int i = 0; i<obj.size(); i++)
			if (getRect().intersects(obj[i].rect))
				if (obj[i].name == "solid")
					if (xSpeed > 0)	
					{ 
						x = obj[i].rect.left - w;
						xSpeed = -0.1; 
						anim.set("left"); 
						anim.play(); 
					}
					else
					{
						x = obj[i].rect.left + obj[i].rect.width;
						xSpeed = 0.1; 
						anim.set("right"); 
						anim.play(); 
					}

		y += ySpeed * time; //Движемся по Y
		ySpeed += 0.0005*time; //Падаем

		for (int i = 0; i<obj.size(); i++)
			if (getRect().intersects(obj[i].rect))
				if (obj[i].name == "solid") //Если столкнулись с объектами
				{
					if (ySpeed > 0)
					{
						y = obj[i].rect.top - h;
						ySpeed = 0;
					}
					else if (ySpeed < 0)
					{
						y = obj[i].rect.top + obj[i].rect.height;
						ySpeed = 0.001;
					}
					break;
				}

		timer += time;
		if (timer > r && ySpeed == 0)
		{
			ySpeed = -0.4;
			timer = 0;
			r = 3000 + rand() % 5000;
		}
		//std::cout << timer << std::endl;

		anim.tick(time);
	}
};

////////////////// Bullet //////////////////

class Bullet
{
private:
	float x1, x2, y1, y2, modul, xSpeed, ySpeed;
	std::vector<Object> obj;
	Sprite blood, bullet;
	int r;
public:
	bool ifFire, ifPopal;
	Sprite s;
	float x, y;

	void setConstructor(Level lev)
	{
		ifPopal = ifFire = false;
		x = y = xSpeed = ySpeed = 0;

		Texture texture;
		obj = lev.GetObjects("solid");
		bullet.setTexture(texture);
		bullet.setTextureRect(IntRect(0, 0, 4, 4));
		bullet.setOrigin(2, 2);
		bullet.setColor(Color::Black);
		blood.setTexture(texture);
		r = rand() % 3 + 2;
		blood.setTextureRect(IntRect(0, 0, r, r));
		blood.setColor(Color::Red);
	}

	void fire(RenderWindow &window, Player &p)
	{
		s = bullet;

		pixelPos = Mouse::getPosition(window);			//забираем коорд курсора
		pos = window.mapPixelToCoords(pixelPos);		//переводим их в игровые

		x1 = pos.x;
		y1 = pos.y;

		x2 = p.x + 15;
		y2 = p.y + 35;

		xSpeed = (x1 - x2);
		ySpeed = (y1 - y2);

		modul = sqrt(xSpeed*xSpeed + ySpeed*ySpeed);

		xSpeed *= 1 / modul;
		ySpeed *= 1 / modul;

		x = p.x + 15;
		y = p.y + 35;

		ifFire = true;
		ifPopal = false;
	}

	void Popal(float x, float y)
	{
		s = blood;
		xSpeed = xSpeed*0.09 + x;
		ySpeed = ySpeed*0.09 + y;
		ifPopal = true;
	}

	FloatRect getRect()
	{
		return FloatRect(x, y, 4, 4);
	}

	void update(float time)
	{
		if (ifPopal) { ySpeed += 0.0001*time; }

		x += xSpeed*time;
		y += ySpeed*time;

		if (!ifPopal)
			for (int i = 0; i < obj.size(); i++)
				if (FloatRect(x, y, 4, 4).intersects(obj[i].rect))
				{
					ifFire = false;
					return;
				}

		if (x2 + 1000 < x || x2 - 1000 > x || y2 + 1000 < y || y2 - 1000 > y)
		{
			ifFire = false;
			return;
		}

		s.setPosition(x, y);
	}
};
