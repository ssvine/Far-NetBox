NetBox: SFTP/FTP(S)/SCP/WebDAV клиент для Far Manager 2.0/3.0
===============

1. Общие сведения о плагине

   Плагин реализует клиентскую часть протоколов SFTP, FTP, SCP, FTPS, WebDAV и S3.  
   SFTP, FTP, SCP, FTPS протоколы реализованы на основе плагина WinSCP [http://winscp.net/eng/download.php/](http://winscp.net/eng/download.php/)  
   Поддержка протокола WebDAV реализована на основе библиотеки neon [http://www.webdav.org/neon/](http://www.webdav.org/neon/).  
   Парсер xml работает c помощью библиотеки TinyXML [http://sourceforge.net/projects/tinyxml/] (http://sourceforge.net/projects/tinyxml/).

2. Использование префикса командной строки

   Доступ к удалённому серверу возможен как через собственное хранилище сессий, так и через префикс.

   Возможны три варианта использования префикса:  
     a. NetBox:Protocol://[[User]:[Password]@]HostName[:Port][/Path]  

        где Protocol - имя протокола (ftp|ftps|ftpes|ssh|scp|sftp|dav|davs|http|https|s3|s3plain)  
            User - имя пользователя  
            Password - пароль пользователя  
            HostName - имя хоста  
            Port - номер порта  
            Path - путь

     b. (sftp|ftp|scp|ftps|webdav|s3)://[[User]:[Password]@]HostName[:Port][/Path]  
        где (sftp|ftp|scp|ftps|webdav|s3) - имя протокола  
            User - имя пользователя  
            Password - пароль пользователя  
            HostName - имя хоста  
            Port - номер порта  
            Path - путь

      c. NetBox:SessionName[/Path]
         где SessionName - сохранённое имя сессии
             Path - путь

    Особенности работы с FTP серверами допускающими анонимный логин.

    ВНИМАНИЕ!

    Если Вы подключаетесь к FTP серверу допускающему анонимный логин,  
    то у Вас возможно сообщение сервера о неудачном логине с  
    использованием пустого пароля. В таком случае вместо анонимного  
    логина попробуйте нормальный логин с использованием пары  
    "логин - пароль" такого вида:

    Логин: Anonymous или для некоторых серверов anonimous (это зависит от настройки данного FTP сервера!)  
    Пароль: user@server.com

    Данная комбинация "имя-пароль" является стандартной формой  
    анонимного логина определённой в спецификациях протокола FTP и  
    допускается на всех разрешающих анонимный логин FTP серверах.


   Например, следующие команды в Far'e позволят просматривать хранилище
   svn с исходными кодами Far:
     a. NetBox: http://farmanager.com/svn/trunk
     b. http://farmanager.com/svn/trunk

3. Ключи при использовании протоколов SFTP, SCP

   Файл с ключами (публичным и приватным) должен быть в формате Putty (.ppk).

4. Фичи

   Комбинация клавиш в панели:  
   Ctrl+Alt+Ins: Копирование текущего URL в буфер обмена (вместе с паролем).  
   Shift+Alt+Ins: Копирование текущего URL в буфер обмена (без пароля).  

5. Установка

   Распакуйте содержимое архива в каталог плагинов Far (...Far\Plugins).

Данный плагин предоставляется "as is" ("как есть"). Автор не несет  
ответственности за последствия использования данного плагина.
