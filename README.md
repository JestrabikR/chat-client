# IPK Projekt 1: Klient pro chat server využívající IPK24-CHAT protokol

## Autor
Radek Jestřabík (xjestr04)

## Překlad
Projekt se překládá pomocí příkazu `make`

## Struktura aplikace
Kód se skládá z hlavní funkce main, která inicializuje sokety a epoll instance, zpracovává vstup od uživatele a zpracovává odpovědi od serveru. Kromě toho obsahuje funkce pro nastavení soketů, obsluhu signálu SIGINT a ošetření chování při chybě. Jednotlivé funkční části jsou rozděleny do vlastních souborů a funkcí, které main následně využívá.

## Zajímavé části implementace
Zde bych rád popsal zajímavé části v implementaci projektu. Ve funkci *main()* se přečte řádek ze standardního vstupu, z něj se získá jaký je to typ příkazu, zavolá se funkce *parse_command()*, která naplní strukturu *Command* daty. Struktura obsahuje typ zprávy, která je v ní obsažena a unii jednotlivých typů zpráv. Tím pádem se může využívat struktura *Command* pro všechny typy zpráv. Zároveň se tímto zpracováním docílí jednotné řešení pro TCP i UDP, protože implementace odeslání zprávy se liší pouze v použití metody, která zprávu odešle.

## Testování
Pro testování jsem primárně využíval WSL Debian s gcc 12.2.0 a nebo dodané referenční vývojové prostředí.
### UDP
Pro testování UDP varianty jsem použil server, který uměl odesílat CONFIRM a REPLY zprávy a zobrazoval, co jsem na něj posílal. Tím jsem si ověřoval funkcionalitu většinu času. Hlavně ze začátku jsem navíc používal wireshark pro ověření co posílám za data. Server také uměl dynamicky měnit port, proto jsem také mohl otestovat, jestli tato funkcionalita funguje správně.\
Testoval jsem:
 - Příkazy, */join*, */auth*, */rename*, */help*
 - Odesílání a přijímání zpráv
 - Vypisování zpráv
 - Potvrzování zpráv pomocí *CONFIRM*
 - Ukončení komunikace pomocí Ctrl-C - odeslání *BYE*
 - Správné chování při výskytu chyby (výpis chyby, uvolnění paměti)
 - Dynamická změna portu
  

### TCP
Pro testování TCP varianty jsem použil vlastní server, který uměl pouze vytvořit TCP spojení a zobrazovat zprávy, které se na něj posílají. 
Testoval jsem:
 - Příkazy, */join*, */auth*, */rename*, */help*
 - Ukončení komunikace pomocí Ctrl-C - odeslání *BYE*
 - Správné chování při výskytu chyby (výpis chyby, uvolnění paměti)