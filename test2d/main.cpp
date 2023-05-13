#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <clocale>
#include <ctime>
#include "level.h"
#include "Entity.h"
#include "anim.h"
#include <vector>
#include <list>
#include <fstream>

using namespace sf;

std::ofstream kills;
int kill;

void Game(String core, RenderWindow &window, View &view)
{
	std::cout << std::endl << "Старт игры." << std::endl;
	//Карта
	Level lvl;
	lvl.LoadFromFile(core + "map.tmx");
	std::cout << "Загрузили карту." << std::endl;

	//создаем время
	Clock c;
	float time;
	time = c.getElapsedTime().asMicroseconds();
	time /= 800;
	std::cout << "Словили время." << std::endl;

	//создаем игрока
	AnimationManager animPlayer;
	Texture player;
	player.loadFromFile(core + "player.png");
	animPlayer.loadFromXML(core + "player.xml", player);
	Font font; //Шрифт 
	font.loadFromFile(core + "a_BighausTitul.ttf");
	Text hpMonitor("", font, 20); //Текст
	Player p(animPlayer, lvl, hpMonitor, time, lvl.GetObject("player").rect.left, lvl.GetObject("player").rect.top);
	std::cout << "Создали игрока." << std::endl;

	//Создаем оружие
	Texture gunTexture;
	gunTexture.loadFromFile(core + "guns.png");
	Gun gun(gunTexture, "ak47");
	Music ak47;
	ak47.openFromFile(core + "ak47.wav");
	ak47.setVolume(3);
	std::cout << "Создаем оружие." << std::endl;

	//создаем пули
	int bulletSize = 50, next = 0;
	Bullet b[50];
	for (int i = 0; i < bulletSize; i++)
		b[i].setConstructor(lvl);
	std::cout << "Создаем пули." << std::endl;

	//Создаем монстра
	AnimationManager animZombie;
	Texture healMonster;
	healMonster.loadFromFile(core + "hp.png");
	Sprite hp;
	hp.setTexture(healMonster);
	hp.setTextureRect(IntRect(0, 130, 26, 10));
	hp.setOrigin(12.5, 0);
	Texture monsterTexture;
	monsterTexture.loadFromFile(core + "zombie.png");
	animZombie.loadFromXML(core + "zombie.xml", monsterTexture);
	std::vector<Object> mobs;
	mobs = lvl.GetObjects("monster");
	std::list<Monster*> m;
	std::list<Monster*>::iterator iterm;
	Monster *iterm2;
	for (int i = 0; i < mobs.size(); i++)
		m.push_back(new Monster(animZombie, lvl, time, mobs[i].rect.left, mobs[i].rect.top + (32 * 2 - 46)));
	std::cout << "Создаем монстров." << std::endl;

	int temp, timer = 0, xCenter, yCenter;

	while (window.isOpen()) //Главный цикл
	{
		time = c.getElapsedTime().asMicroseconds();
		c.restart();
		time /= 800; //speed game
		if (time > 40) time = 40;

		Event event;
		while (window.pollEvent(event))
		{
			if (event.type == Event::Closed)
			{
				std::cout << "Удаляем все." << std::endl;
				for (iterm = m.begin(); iterm != m.end();)
				{
					iterm2 = *iterm;
					m.erase(iterm++);
					delete iterm2;
				}
				std::cout << "Закрывем игру." << std::endl;
				kills << kill;
				kills.close();
				window.close();
				return;
			}

			/*if (event.type == Event::MouseButtonPressed)
			if (event.key.code == Mouse::Left)
			{
			ak47.play();
			b.push_back(new Bullet(lvl, window, p));
			}*/
		}

		timer += time;
		if (Mouse::isButtonPressed(Mouse::Left) && timer > 100)
		{
			ak47.play();
			b[next].fire(window, p);
			next++;
			if (next >= bulletSize) next = 0;
			timer = 0;
		}

		//Чистим экран
		window.clear(Color::Cyan);

		//Все камеры на игрока
		if (p.y > H - 21)
			yCenter = H - 21;
		else if (p.y < 273)
			yCenter = 273;
		else
			yCenter = p.y;
		if (p.x > 1099)
			xCenter = 1099;
		else if (p.x < 496)
			xCenter = 496;
		else
			xCenter = p.x;
		view.setCenter(xCenter, yCenter);
		window.setView(view);

		//Обнова карты
		lvl.Draw(window);

		//обновление игрока
		p.update(time);
		p.draw(window);
		if (p.heal < 0)
			return;

		//Обновляем монстров
		for (iterm = m.begin(); iterm != m.end();)
			if ((*iterm)->isLife)
			{
				(*iterm)->update(time);
				(*iterm)->draw(window);
				hp.setTextureRect(IntRect(0, ((int)((*iterm)->heal / 7.143)) * 10, 26, 10));
				hp.setPosition((*iterm)->x + (*iterm)->w / 2, (*iterm)->y - 15);
				window.draw(hp);
				if ((*iterm)->getRect().intersects(p.getRect()))
					p.attack();
				else p.isAttack = true;
				iterm++;
			}
			else
			{
				iterm2 = *iterm;
				m.erase(iterm++);
				delete iterm2;
				temp = rand() % mobs.size();
				m.push_back(new Monster(animZombie, lvl, time, mobs[temp].rect.left, mobs[temp].rect.top + (32 * 2 - 46)));
				temp = rand() % mobs.size();
				m.push_back(new Monster(animZombie, lvl, time, mobs[temp].rect.left, mobs[temp].rect.top + (32 * 2 - 46)));
				p.score++;
				if (p.score > kill)
					kill = p.score;
			}

		//Обновление пулей
		for (int i = 0; i < bulletSize; i++)
		{
			if (b[i].ifFire)
			{
				for (iterm = m.begin(); iterm != m.end(); iterm++)
					if (!b[i].ifPopal && (*iterm)->getRect().intersects(b[i].getRect()))
					{
						(*iterm)->attack();
						b[i].Popal((*iterm)->xSpeed, (*iterm)->ySpeed);
					}

				b[i].update(time);
				window.draw(b[i].s);
			}
		}

		//Обновляем оружие
		gun.update(time, window, p);
		window.draw(gun.gun);

		//Выводим всякие данные
		p.hpMonitor.setPosition(view.getCenter().x - W / 2 + 10, view.getCenter().y - H / 2);
		window.draw(p.hpMonitor);

		//Вывод всего
		window.display();
	}

	std::cout << "Удаляем все." << std::endl;
	for (iterm = m.begin(); iterm != m.end();)
	{
		iterm2 = *iterm;
		m.erase(iterm++);
		delete iterm2;
	}
	std::cout << "Конец игры." << std::endl;
}

void Menu(String core, RenderWindow &window, View &view)
{
	std::cout << std::endl << "Выводим меню." << std::endl;

	Texture logo;
	logo.loadFromFile(core + "logo.png");
	Sprite logoS;
	logoS.setTexture(logo);
	logoS.setPosition(W / 2 - logoS.getTextureRect().width / 2, H / 2 - logoS.getTextureRect().height / 2 - 32 * 5);

	Texture play;
	play.loadFromFile(core + "buttonPlay.png");
	Sprite playS;
	playS.setTexture(play);
	playS.setPosition(W / 2 - playS.getTextureRect().width / 2, H / 2 - playS.getTextureRect().height / 2 - 50);

	Texture close;
	close.loadFromFile(core + "buttonStop.png");
	Sprite closeS;
	closeS.setTexture(close);
	closeS.setPosition(W / 2 - closeS.getTextureRect().width / 2, H / 2 - closeS.getTextureRect().height / 2 + 50);

	view.setCenter(496,272);
	window.setView(view);

	Font font; //Шрифт 
	font.loadFromFile(core + "a_BighausTitul.ttf");
	Text hpMonitor("", font, 20); //Текст
	hpMonitor.setColor(Color::Red);
	hpMonitor.setPosition(W / 2 + 150, H / 2 + 150);
	hpMonitor.setString("Рекорд убийств: " + std::to_string(kill));

	while (true)
	{
		pixelPos = Mouse::getPosition(window);		//забираем коорд курсора
		pos = window.mapPixelToCoords(pixelPos);	//переводим их в игровые

		Event event;
		while (window.pollEvent(event))
		{
			if (event.type == Event::Closed)
			{
				window.close();
				kills << kill;
				kills.close();
				return;
			}

			if (event.type == Event::MouseButtonPressed)
				if (event.key.code == Mouse::Left)
					if (playS.getGlobalBounds().contains(pos.x, pos.y))
						return;
					else if (closeS.getGlobalBounds().contains(pos.x, pos.y))
					{
						window.close();

						kills << kill;
						kills.close();
					}
		}

		if (playS.getGlobalBounds().contains(pos.x, pos.y))
		{
			playS.setColor(Color::Red);
		}
		else playS.setColor(Color::White);

		if (closeS.getGlobalBounds().contains(pos.x, pos.y))
		{
			closeS.setColor(Color::Red);
		}
		else closeS.setColor(Color::White);

		//Чистим экран
		window.clear(Color::Cyan);

		//Вывод лого
		window.draw(logoS);

		//Вывод кнопки
		window.draw(playS);
		window.draw(closeS);

		//вывод рекорда
		window.draw(hpMonitor);

		//Вывод всего
		window.display();
	}
}

int main()
{
	setlocale(LC_CTYPE, "rus");
	srand(time(0));
	RenderWindow window(VideoMode(W, H), "SkoloShot v2.1"); //Создали окно
	View view(FloatRect(0, 0, W, H));

	String core = "FilesMyGame/"; //Дериктория с файлами

	std::fstream readKill;
	readKill.open("FilesMyGame/kills.txt");
	readKill >> kill;
	readKill.close();
	
	kills.open("FilesMyGame/kills.txt", std::ios::trunc);
	
	while (window.isOpen())
	{
		if (window.isOpen()) Menu(core, window, view);
		if (window.isOpen()) Game(core, window, view);
	}

	
	kills << kill;
	kills.close();

	window.close();
	std::cout << "Закрываем прогу." << std::endl;
	return 0;
}

