#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>

sf::Font defaultFont;
sf::SoundBuffer defaultSound;

int main()
{
    defaultFont.loadFromFile("rsc/font/RobotoCondensed-Regular.ttf");
    defaultSound.loadFromFile("rsc/sound/thud.wav");
    sf::Sound sound;
    sound.setBuffer(defaultSound);

    sf::RenderWindow window(sf::VideoMode(800, 600), "My window");

    while(window.isOpen())
    {
        sf::Event event;
        while(window.pollEvent(event))
        {
            if(event.type == sf::Event::Closed)
                window.close();
            else if(event.type == sf::Event::KeyPressed)
            {
                if(event.key.code == sf::Keyboard::S)
                {
                    sound.play();
                    std::cout << "Lel" << std::endl;
                }
            }
        }

        window.clear(sf::Color(77, 77, 77));

        sf::Text text("Engmsc APPsdfsdf", defaultFont);
        
        window.draw(text);

        window.display();
    }

    return 0;
}