# script to create the html files needed

#webserv/
#├── www/             # static files go here
#│   └── index.html
#├── uploads/         # File uploads will be stored here
#├── cgi-bin/         # CGI scripts (e.g., .py, .php) go here
#│   └── hello.py
#└── server.conf      # cconf files

# at root
rm -rf ./www
rm -rf ./uploads
rm -rf ./cgi-bin
rm -rf ./public

mkdir -p ./www
mkdir -p ./www/errors
mkdir -p ./www/api
mkdir -p ./www/default
mkdir -p ./uploads
mkdir -p ./cgi-bin
mkdir -p ./public

chmod -R 755 ./



# default python script for CGI
cat << 'EOF' > ./cgi-bin/hello.py
#!/usr/bin/env python3

import os
import sys
import platform

method = os.environ.get('REQUEST_METHOD', '')
body = ''
if method == 'POST':
    try:
        content_length = int(os.environ.get('CONTENT_LENGTH', 0))
        body = sys.stdin.read(content_length)
    except:
        body = '[couldn’t read POST data]'

# env info
os_name = platform.system()
os_version = platform.version()
python_version = platform.python_version()
cwd = os.getcwd()
user = os.environ.get('USER') or os.environ.get('USERNAME') or 'Unknown'

print("Content-Type: text/plain\r\n")

print("✅ CGI script executed successfully!\n")
print(f"👤 User: {user}")
print(f"📦 Python version: {python_version}")
print(f"🖥 OS: {os_name} {os_version}")
print(f"📂 Current working directory: {cwd}")
print(f"🧾 Method: {method}")
print(f"📨 POST body: {body}")
while True:
    pass;
EOF

chmod +x ./cgi-bin/hello.py
# curl http://localhost:8080/cgi-bin/hello.py
# curl -X POST -d "foo=bar&baz=qux" http://localhost:8080/cgi-bin/hello.py




# Ruby CGI script
cat << 'EOF' > ./cgi-bin/hello.rb
#!/usr/bin/env ruby

require 'cgi'
require 'etc'

cgi = CGI.new
method = ENV['REQUEST_METHOD'] || ''
body = ''

# sys info
ruby_version = RUBY_VERSION
os_name = RUBY_PLATFORM
cwd = Dir.pwd
user = Etc.getlogin || ENV['USER'] || ENV['USERNAME'] || 'Unknown'

puts "Content-Type: text/plain\r\n"

puts "✅ CGI script executed successfully!\n\n"
puts "👤 User: #{user}"
puts "💎 Ruby version: #{ruby_version}"
puts "🖥 Platform: #{os_name}"
puts "📂 Current working directory: #{cwd}"
puts "🧾 Method: #{method}"
EOF

# Make it executable
chmod +x ./cgi-bin/hello.rb
# curl "http://localhost:8080/cgi-bin/hello.rb?name=zeu"



# create index.html with purple theme
cat << 'EOF' > ./www/index.html
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>Webserv.cpp</title>
    <style>
        body {
            margin: 0;
            padding: 0;
            font-family: "Segoe UI", sans-serif;
            background: linear-gradient(135deg, #3f0f4f, #7e57c2);
            color: #f3e5f5;
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            height: 100vh;
        }

        h1 {
            color: #e1bee7;
            font-size: 3rem;
            margin-bottom: 0.5rem;
            text-shadow: 1px 1px #2c003e;
        }

        p {
            font-size: 1.2rem;
            color: #f8eafa;
        }

        .card {
            background: rgba(63, 15, 79, 0.85);
            padding: 2rem 3rem;
            border-radius: 16px;
            box-shadow: 0 8px 24px rgba(0,0,0,0.3);
            text-align: center;
            animation: float 3s ease-in-out infinite;
        }

        @keyframes float {
            0% { transform: translateY(0); }
            50% { transform: translateY(-8px); }
            100% { transform: translateY(0); }
        }

        .links {
            margin-top: 2rem;
        }

        .links a, .links form button {
            display: inline-block;
            margin: 0.5rem;
            padding: 0.75rem 1.5rem;
            background: #ba68c8;
            color: white;
            text-decoration: none;
            border-radius: 8px;
            font-weight: bold;
            transition: background 0.3s ease;
        }

        .links a:hover, .links form button:hover {
            background: #ab47bc;
        }

        .links form {
            display: inline;
        }

        .links form button {
            background: none;
            border: none;
            padding: 0.75rem 1.5rem;
            color: #bb86fc;
            text-decoration: underline;
            cursor: pointer;
        }

        .ports {
            margin-top: 3rem;
        }

        .ports h3 {
            margin-bottom: 1rem;
            color: #d1c4e9;
        }

        .ports a {
            display: inline-block;
            margin: 0.4rem;
            padding: 0.5rem 1.2rem;
            background: #9575cd;
            color: white;
            text-decoration: none;
            border-radius: 6px;
            transition: background 0.3s ease;
        }
        .lk a{
            border: 1px solid white;
        }

        .ports a:hover {
            background: #7e57c2;
        }

        h4, h5 {
            margin-top: 1rem;
            color: #f3e5f5;
        }

        h5 a {
            color: #d1c4e9;
            margin-left: 0.5rem;
        }

        h5 a:hover {
            text-decoration: underline;
        }
    </style>
</head>
<body>
    <h4>Jsaintho et Gchauvot presents:</h4>
    <div class="card">
        <h1>42 WEBSERVS GOES BRRRRRRRRR!</h1>
        <p>Cette page is served by your <strong>webserv</strong> C++98 HTTP server.</p>
        <div class="links">
            <a href="/non-exzeistent">404</a>
            <a href="/null">403 no-autoindex</a>
            <a href="/files">403 autoindex-on (root files listing)</a>
            <form action="/files" method="POST" onsubmit="return confirm('trigger 405 error with POST?');">
                <button type="submit">trigger le 405 (POST not allowed)</button>
            </form>
        </div>
        <div class="links lk">
            <a href="/upload">UPLOAD +</a>
            <a href="/delete-upload">DELETE -</a>
            <a href="/page-cgi">CGI Endpoint</a>
        </div>
        <div class="ports">
            <h3>Available Servers</h3>
            <a href="http://127.0.0.1:8088" target="_blank">localhost:8088 (here)</a>
            <a href="http://127.0.0.1:8082" target="_blank">apiABC.com:8082</a>
            <a href="http://127.0.0.1:9091" target="_blank">campton:9091</a>
            <a href="http://127.0.0.1:4343" target="_blank">special</a>
        </div>
        <div class="ports">
            <h3>homies</h3>
            <a href="https://google.com" target="_blank">GOOGLE</a>
            <a href="https://facebook.com" target="_blank">FACEBOOK</a>
            <a href="/call-redirect">our redirect to google</a>
        </div>
    </div>
    <h5>all rights reserved.
        <a href="https://github.com/youradrien">le j</a>
    </h5>
</body>
</html>
EOF






# create 404.html
cat << 'EOF' > ./www/errors/404.html
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>404 - Page Not Found</title>
    <style>
        @keyframes float {
            0% { transform: translateY(0); }
            50% { transform: translateY(-8px); }
            100% { transform: translateY(0); }
        }

        body {
            margin: 0;
            padding: 0;
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #8e0e00, #e52e71);
            color: #ffebee;
            display: flex;
            align-items: center;
            justify-content: center;
            height: 100vh;
            overflow: hidden;
        }

        .container {
            text-align: center;
            animation: float 3s ease-in-out infinite;
        }

        h1 {
            font-size: 6rem;
            margin: 0;
            color: #ffcdd2;
            text-shadow: 2px 2px #b71c1c;
        }

        h2 {
            color: #ffcccb;
        }

        h4 {
            font-size: 1.2rem;
            color: #f8bbd0;
            max-width: 600px;
            margin: 1.5rem auto;
            line-height: 1.6;
        }

        p {
            font-size: 1.2rem;
            margin-top: 1rem;
            color: #ffdde1;
        }

        a {
            display: inline-block;
            margin-top: 1.5rem;
            padding: 0.5rem 1rem;
            background-color: #ef5350;
            color: #fff;
            text-decoration: none;
            border-radius: 8px;
            font-weight: bold;
            transition: background 0.3s ease;
        }

        a:hover {
            background-color: #e53935;
        }

        .emoji {
            font-size: 3rem;
            margin-top: 2rem;
            animation: float 2s ease-in-out infinite;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>404</h1>
        <h2>NON-default 404</h2>
        <h4>
            couldn't find what you're looking for.<br>
            Maybe the URL is wrong, or the resource just vanished into the void.<br>
            Happens to the best of us.
        </h4>
        <p>You’ve drifted off course... 🔥</p>
        <a href="http://127.0.0.1:8088">Return to safety</a>
        <div class="emoji">🚨🧯🛑</div>
    </div>
</body>
</html>
EOF


# DEFAULT blue-themed 404.html
cat << 'EOF' > ./www/default/404.html
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>404 - Page Not Found</title>
    <style>
        @keyframes float {
            0% { transform: translateY(0); }
            50% { transform: translateY(-8px); }
            100% { transform: translateY(0); }
        }

        body {
            margin: 0;
            padding: 0;
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #1e3c72, #2a5298);
            color: #e0f7fa;
            display: flex;
            align-items: center;
            justify-content: center;
            height: 100vh;
            overflow: hidden;
        }

        .container {
            text-align: center;
            animation: float 3s ease-in-out infinite;
        }

        h1 {
            font-size: 6rem;
            margin: 0;
            color: #bbdefb;
            text-shadow: 2px 2px #0d47a1;
        }

        h4 {
            font-size: 1.2rem;
            color: #b3e5fc;
            max-width: 600px;
            margin: 1.5rem auto;
            line-height: 1.6;
        }

        p {
            font-size: 1.2rem;
            margin-top: 1rem;
            color: #e3f2fd;
        }

        a {
            display: inline-block;
            margin-top: 1.5rem;
            padding: 0.5rem 1rem;
            background-color: #64b5f6;
            color: #0d47a1;
            text-decoration: none;
            border-radius: 8px;
            font-weight: bold;
            transition: background 0.3s ease;
        }

        a:hover {
            background-color: #42a5f5;
        }

        .emoji {
            font-size: 3rem;
            margin-top: 2rem;
            animation: float 2s ease-in-out infinite;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>404</h1>
        <h2>DEFAULT 404</h2>
        <h4>
            The server couldn't find what you're looking for.<br>
            Maybe the URL is wrong, or the resource just vanished into the void.<br>
            Happens to the best of us.
        </h4>
        <p>You’ve drifted off course... 🚧</p>
        <a href="http://127.0.0.1:8088">Return to safety</a>
        <div class="emoji">🌊🛰️🔵</div>
    </div>
</body>
</html>
EOF





# create blue-themed 403.html
cat << 'EOF' > ./www/default/403.html
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>403 - Forbidden</title>
    <style>
        @keyframes pulse {
            0% { transform: scale(1); opacity: 1; }
            50% { transform: scale(1.05); opacity: 0.8; }
            100% { transform: scale(1); opacity: 1; }
        }

        body {
            margin: 0;
            padding: 0;
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #1a2980, #26d0ce);
            color: #e0f7fa;
            display: flex;
            align-items: center;
            justify-content: center;
            height: 100vh;
            overflow: hidden;
        }

        .container {
            text-align: center;
            animation: pulse 4s ease-in-out infinite;
        }

        h1 {
            font-size: 6rem;
            margin: 0;
            color: #b3e5fc;
            text-shadow: 2px 2px #0d47a1;
        }

        h4 {
            font-size: 1.3rem;
            color: #e3f2fd;
            max-width: 600px;
            margin: 1.5rem auto;
            line-height: 1.6;
        }

        p {
            font-size: 1.1rem;
            color: #bbdefb;
        }

        a {
            display: inline-block;
            margin-top: 1.5rem;
            padding: 0.6rem 1.2rem;
            background-color: #4fc3f7;
            color: #003c8f;
            text-decoration: none;
            border-radius: 8px;
            font-weight: bold;
            transition: background 0.3s ease;
        }

        a:hover {
            background-color: #29b6f6;
        }

        .emoji {
            font-size: 3rem;
            margin-top: 2rem;
            animation: pulse 2s ease-in-out infinite;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>403</h1>
        <h4>
            You don't have permission to access this resource.<br>
            Maybe you took a wrong turn, or this zone is locked 🔐.
        </h4>
        <p>Security drones are watching... 🛡️</p>
        <a href="http://127.0.0.1:8088">Back to safer ground</a>
        <div class="emoji">🚫🧊🔐</div>
    </div>
</body>
</html>
EOF




# create 404.html
cat << 'EOF' > ./www/api.html
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>API (apiABC.com:8082)</title>
    <style>
        @keyframes float {
            0% { transform: translateY(0); }
            50% { transform: translateY(-10px); }
            100% { transform: translateY(0); }
        }

        body {
            margin: 0;
            padding: 0;
            font-family: 'Fredoka', sans-serif;
            background: linear-gradient(135deg, #ff9a9e 0%, #fad0c4 100%);
            color: #fff;
            display: flex;
            align-items: center;
            justify-content: center;
            height: 100vh;
            overflow: hidden;
        }

        .container {
            text-align: center;
            animation: float 3s ease-in-out infinite;
        }

        h1 {
            font-size: 5rem;
            margin: 0;
            color: #ffffff;
            text-shadow: 2px 2px #ff6f91;
        }

        p {
            font-size: 1.5rem;
            margin-top: 1rem;
            color: #fff0f5;
        }

        .emoji {
            font-size: 3rem;
            margin-top: 2rem;
            animation: float 2s ease-in-out infinite;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>Welcome to API_ABC</h1>
        <h2>
            this location is just made to render/simulate api call
        </h2>
        <p>42</p>
        <button>
                <a href="/api">check api logs 🌌</a>
        </button>
        <button>
                <a href="http://localhost:8088/">retour à la maison</a>
        </button>
        <div class="emoji">🛸👾🚀</div>
    </div>
</body>
</html>
EOF




# create 403.html non - autoindexer
cat << 'EOF' > ./www/errors/403.html
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>403 - Forbidden</title>
    <style>
        @keyframes shake {
            0%, 100% { transform: translateX(0); }
            25% { transform: translateX(-10px); }
            50% { transform: translateX(10px); }
            75% { transform: translateX(-5px); }
        }

        body {
            margin: 0;
            padding: 0;
            font-family: "Comic Sans MS", cursive, sans-serif;
            background: linear-gradient(135deg, #a18cd1 0%, #fbc2eb 100%);
            color: #fff;
            display: flex;
            align-items: center;
            justify-content: center;
            height: 100vh;
            overflow: hidden;
        }

        .container {
            text-align: center;
            animation: shake 1.5s ease-in-out infinite;
        }

        h1 {
            font-size: 5rem;
            margin: 0;
            color: #ffffff;
            text-shadow: 2px 2px #c471ed;
        }

        p {
            font-size: 1.5rem;
            margin-top: 1rem;
            color: #f0e6ff;
        }

        .emoji {
            font-size: 3rem;
            margin-top: 2rem;
        }

        a {
            display: inline-block;
            margin-top: 2rem;
            padding: 0.5rem 1rem;
            background: #ffffff20;
            border-radius: 10px;
            color: white;
            text-decoration: none;
            backdrop-filter: blur(4px);
            transition: background 0.3s ease;
        }

        a:hover {
            background: #ffffff40;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>403</h1>
        <h5>client does not have access rights to the content; that is, it is unauthorized, so the server is refusing to give the requested resource.</h5>
        <p>nice try... but you can't go there 🚫</p>
        <a href="/">back to safety</a>
        <div class="emoji">🧙‍♂️🔒🛡️</div>
    </div>
</body>
</html>
EOF




# create 405.html non - autoindexer
cat << 'EOF' > ./www/errors/405.html
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>405 - Method Not Allowed</title>
    <style>
        @keyframes shake {
            0%, 100% { transform: translateX(0); }
            25% { transform: translateX(-10px); }
            50% { transform: translateX(10px); }
            75% { transform: translateX(-5px); }
        }

        body {
            margin: 0;
            padding: 0;
            font-family: "Comic Sans MS", cursive, sans-serif;
            background: linear-gradient(135deg, #a18cd1 0%, #fbc2eb 100%);
            color: #fff;
            display: flex;
            align-items: center;
            justify-content: center;
            height: 100vh;
            overflow: hidden;
        }

        .container {
            text-align: center;
            animation: shake 1.5s ease-in-out infinite;
        }

        h1 {
            font-size: 5rem;
            margin: 0;
            color: #ffffff;
            text-shadow: 2px 2px #c471ed;
        }

        p {
            font-size: 1.5rem;
            margin-top: 1rem;
            color: #f0e6ff;
        }

        .emoji {
            font-size: 3rem;
            margin-top: 2rem;
        }

        a {
            display: inline-block;
            margin-top: 2rem;
            padding: 0.5rem 1rem;
            background: #ffffff20;
            border-radius: 10px;
            color: white;
            text-decoration: none;
            backdrop-filter: blur(4px);
            transition: background 0.3s ease;
        }

        a:hover {
            background: #ffffff40;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>405 - Method Not Allowed</h1>
        <h3>Tried wrong method on this route ?</h3>
        <h5>the request method is known by the server but is not supported by the target resource.
        for example, an API may not allow DELETE on a resource, or the TRACE method entirely.</h5>
        <a href="/">go back </a>
        <div class="emoji">🔒🛡️</div>
    </div>
</body>
</html>
EOF






# create 403.html autoindexer
# create index.html
cat << 'EOF' > ./www/errors/autoindex.html
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>403 Webserv.cpp autoindex</title>
    <style>
        body {
            margin: 0;
            padding: 0;
            font-family: "Segoe UI", sans-serif;
            background: #f4f4f4;
            color: #333;
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            height: 100vh;
        }
        h1 {
            color: #007acc;
            font-size: 3rem;
            margin-bottom: 0.5rem;
        }
        p {
            font-size: 1.2rem;
            color: #555;
        }
        .card {
            background: white;
            padding: 2rem 3rem;
            border-radius: 12px;
            box-shadow: 0 5px 15px rgba(0,0,0,0.1);
            text-align: center;
        }
        .links {
            margin-top: 2rem;
        }
        .links a {
            display: inline-block;
            margin: 0.5rem;
            padding: 0.75rem 1.5rem;
            background: #007acc;
            color: white;
            text-decoration: none;
            border-radius: 8px;
            transition: background 0.3s ease;
        }
        .links a:hover {
            background: #005f99;
        }
    </style>
</head>
<body>
    <div class="card">
        <h1>webserver.cpp AUTOINDXER</h1>
        <p>cette page list files et folders dans /var/www .</p>
        <h1>directory listing :</h1>
        <ul>
        <!--CONTENT-->
        </ul>
        <h5>
            <a href="/">retour</a>
        </h5>
    </div>
</body>
</html>
EOF

cat << 'EOF' >> ./www/campton.html
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>Welcome to Campton, USA</title>
    <style>
        body {
            background: #e6f2ff;
            margin: 0;
            padding: 0;
            display: flex;
            flex-direction: column;
            align-items: center;
            min-height: 100vh;
            text-align: center;
            color: #333;
        }
        header {
            background: #005f99;
            color: white;
            width: 100%;
            padding: 2rem 0;
        }
        main {
            padding: 2rem;
        }
        h1 {
            margin: 0;
            font-size: 3rem;
        }
        p {
            font-size: 1.2rem;
            margin: 1rem 0;
            max-width: 600px;
        }
        img {
            margin-top: 2rem;
            max-width: 80%;
            height: auto;
            border-radius: 10px;
            box-shadow: 0 0 10px rgba(0,0,0,0.3);
        }
        footer {
            margin-top: auto;
            padding: 1rem;
            background: #ddd;
            width: 100%;
        }
    </style>
</head>
<body>
    <header>
        <h1>Welcome to Campton, USA</h1>
    </header>
    <main>
        <p>
            Nestled in the heart of the countryside, Campton is a small town with a big heart.
            Whether you're here for the scenic views, local hospitality, or just passing through —
            there's always something to enjoy in Campton.
        </p>
        <p>
            From hiking trails and historic buildings to cozy diners and friendly faces,
            Campton invites you to slow down and soak in the charm of small-town America.
        </p>
        <img src="https://upload.wikimedia.org/wikipedia/commons/8/8e/Hood_Camptown_Scene.jpg" alt="Hood Campton" />
    </main>
    <footer>
        2025 Campton Tourism Board
    </footer>
</body>
</html>
EOF

# create SPECIAL.html
cat << 'EOF' > ./public/indexx.html
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8" />
  <title>SPECIAL!!!!</title>
  <style>
    html, body {
      margin: 0;
      padding: 0;
      height: 100%;
      overflow: hidden;
      font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
      color: white;
      background: linear-gradient(270deg, #1a0040, #000033, #2e0055, #000044);
      background-size: 800% 800%;
      animation: bgColorShift 3s ease infinite;
    }

    @keyframes bgColorShift {
      0% {background-position: 0% 50%;}
      50% {background-position: 100% 50%;}
      100% {background-position: 0% 50%;}
    }

    canvas {
      position: absolute;
      top: 0;
      left: 0;
      z-index: 0;
      display: block;
    }

    .content {
      position: relative;
      z-index: 1;
      height: 100%;
      display: flex;
      align-items: center;
      justify-content: center;
      flex-direction: column;
      text-align: center;
    }

    h1 {
      font-size: 4rem;
      letter-spacing: 0.1em;
      color: #ffffffcc;
      text-shadow: 0 0 15px #aaffff, 0 0 30px #33ccff;
      animation: glow 3s ease-in-out infinite alternate;
    }

    @keyframes glow {
      from {
        text-shadow: 0 0 15px #aaffff, 0 0 30px #33ccff;
      }
      to {
        text-shadow: 0 0 25px #ffffff, 0 0 40px #66ffff;
      }
    }
  </style>
</head>
<body>
  <canvas id="stars"></canvas>
  <div class="content">
    <h1>something special</h1>

    <h1><a href=http://127.0.0.1:8088/>HOME</a></h1>
    <h3>
        <a href="/null">another 403</a>
    </h3>
    <h3>
        <a href="/ezrrrr'__'">default 404</a>
    </h3>
  </div>

  <script>
    const canvas = document.getElementById('stars');
    const ctx = canvas.getContext('2d');

    // Resize canvas to fill window
    function resize() {
      canvas.width = window.innerWidth;
      canvas.height = window.innerHeight;
    }
    window.addEventListener('resize', resize);
    resize();

    // Create stars
    const starCount = 150;
    let stars = [];
    for (let i = 0; i < starCount; i++) {
      stars.push({
        x: Math.random() * canvas.width,
        y: Math.random() * canvas.height,
        radius: Math.random() * 1.2 + 0.3,
        speed: Math.random() * 0.5 + 0.2,
        alpha: Math.random() * 0.8 + 0.2
      });
    }

    function drawStars() {
      ctx.clearRect(0, 0, canvas.width, canvas.height);
      ctx.fillStyle = '#ffffff';
      stars.forEach(star => {
        ctx.globalAlpha = star.alpha;
        ctx.beginPath();
        ctx.arc(star.x, star.y, star.radius, 0, Math.PI * 2);
        ctx.fill();

        star.y += star.speed;
        if (star.y > canvas.height) {
          star.y = 0;
          star.x = Math.random() * canvas.width;
        }
      });
      ctx.globalAlpha = 1.0;
      requestAnimationFrame(drawStars);
    }

    drawStars();
  </script>
</body>
</html>
EOF




# create index.json
cat << 'EOF' > ./www/api/index.json
{
  {
    "alpha": "92",
    "bravo": "sunset",
    "charlie": "1098",
    "delta": "ocean",
    "echo": "42",
    "foxtrot": "cactus",
    "golf": "7831",
    "hotel": "cloud",
    "india": "556",
    "juliet": "zebra",
    "kilo": "3021",
    "lima": "banana",
    "mike": "88",
    "november": "forest",
    "oscar": "719",
    "papa": "river",
    "quebec": "1045",
    "romeo": "mountain",
    "sierra": "301",
    "tango": "tiger",
    "uniform": "876",
    "victor": "maple",
    "whiskey": "99",
    "xray": "galaxy",
    "yankee": "212",
    "zulu": "echo",
    "omega": "4545",
    "theta": "pine",
    "sigma": "60",
    "lambda": "horizon"
  }
}
EOF





# create postz.html
cat << 'EOF' > ./www/postz.html
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>Upload - Webserver.cpp</title>
    <style>
        @keyframes float {
            0% { transform: translateY(0); }
            50% { transform: translateY(-8px); }
            100% { transform: translateY(0); }
        }

        body {
            margin: 0;
            padding: 0;
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #004d40, #26a69a);
            color: #e0f2f1;
            display: flex;
            align-items: center;
            justify-content: center;
            height: 100vh;
            overflow: hidden;
        }

        .container {
            text-align: center;
            background-color: rgba(0, 77, 64, 0.8);
            padding: 3rem;
            border-radius: 16px;
            box-shadow: 0 8px 24px rgba(0,0,0,0.3);
            animation: float 3s ease-in-out infinite;
        }

        h1 {
            font-size: 3rem;
            margin-bottom: 1rem;
            color: #b2dfdb;
            text-shadow: 1px 1px #00251a;
        }

        p {
            font-size: 1.2rem;
            color: #a7ffeb;
        }

        form {
            margin-top: 2rem;
        }

        label {
            font-size: 1rem;
            font-weight: bold;
            display: block;
            margin-bottom: 0.5rem;
        }

        input[type="file"] {
            padding: 0.5rem;
            margin-bottom: 1rem;
            background-color: #e0f2f1;
            border-radius: 6px;
            border: none;
        }

        #filenameDisplay {
            font-size: 1rem;
            margin-bottom: 1rem;
            color: #b2dfdb;
            font-style: italic;
        }

        button {
            padding: 0.75rem 1.5rem;
            background-color: #4db6ac;
            color: #004d40;
            border: none;
            border-radius: 10px;
            font-weight: bold;
            font-size: 1rem;
            cursor: pointer;
            transition: background 0.3s ease;
        }

        button:hover {
            background-color: #26a69a;
        }

        a {
            display: inline-block;
            margin-top: 1.5rem;
            padding: 0.5rem 1rem;
            background-color: #80cbc4;
            color: #004d40;
            text-decoration: none;
            border-radius: 8px;
            font-weight: bold;
            transition: background 0.3s ease;
        }

        a:hover {
            background-color: #4db6ac;
        }

        .emoji {
            font-size: 2.5rem;
            margin-top: 1rem;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>File Upload</h1>
        <p>Upload a file to <code>/upload</code></p>

        <form id="uploadForm" action="/upload" method="post" enctype="multipart/form-data" autocomplete="off">
            <label for="fileUpload">📁 Pick a file</label>
            <input type="file" id="fileUpload" name="file" required />
            <div id="filenameDisplay">No file selected</div>
            <button type="submit">🚀 Upload</button>
        </form>
        <div id="uploadStatus" style="margin-top: 1rem; font-weight: bold;"></div>

        <a href="/">← Return Home</a>
        <div class="emoji">🌿📤</div>
    </div>

    <script>
        const fileInput = document.getElementById('fileUpload');
        const fileNameDisplay = document.getElementById('filenameDisplay');
        const uploadForm = document.getElementById('uploadForm');
        const statusDiv = document.getElementById('uploadStatus');

        fileInput.addEventListener('change', function() {
            if (fileInput.files.length > 0) {
                fileNameDisplay.textContent = `Selected: ${fileInput.files[0].name}`;
            } else {
                fileNameDisplay.textContent = "No file selected";
            }
        });

        uploadForm.addEventListener('submit', async function(e) {
            e.preventDefault(); // Stop default form submit

            const formData = new FormData(uploadForm);

            try {
                const res = await fetch("/upload", {
                    method: "POST",
                    body: formData
                });

                if (res.ok) {
                    statusDiv.textContent = "✅ Fichier uploadé avec succès !";
                    statusDiv.style.color = "#a7ffeb";
                } else {
                    statusDiv.textContent = "❌ Erreur lors de l'upload.";
                    statusDiv.style.color = "salmon";
                }
            } catch (err) {
                statusDiv.textContent = "❌ Une erreur réseau est survenue.";
                statusDiv.style.color = "red";
            }
        });
        </script>
</body>
</html>
EOF






# create deletz.html
cat << 'EOF' > ./www/deletz.html
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8" />
    <title>Delete Files - Webserver.cpp</title>
    <style>
        @keyframes float {
            0% { transform: translateY(0); }
            50% { transform: translateY(-8px); }
            100% { transform: translateY(0); }
        }

        body {
            margin: 0;
            padding: 0;
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #4a148c, #7b1fa2);
            color: #f3e5f5;
            display: flex;
            align-items: center;
            justify-content: center;
            height: 100vh;
            overflow: hidden;
        }

        .container {
            text-align: center;
            background-color: rgba(74, 20, 140, 0.85);
            padding: 3rem;
            border-radius: 16px;
            box-shadow: 0 8px 24px rgba(0,0,0,0.3);
            animation: float 3s ease-in-out infinite;
            max-height: 90vh;
            overflow-y: auto;
            width: 320px;
        }

        h1 {
            font-size: 2.5rem;
            margin-bottom: 1.5rem;
            color: #ce93d8;
            text-shadow: 1px 1px #12005e;
        }

        form {
            margin-bottom: 1rem;
        }

        input[type="text"] {
            width: 100%;
            padding: 0.5rem;
            font-size: 1rem;
            border-radius: 8px;
            border: none;
            margin-bottom: 0.8rem;
        }

        button {
            padding: 0.5rem 1rem;
            background-color: #ab47bc;
            color: white;
            border: none;
            border-radius: 8px;
            font-weight: bold;
            font-size: 1rem;
            cursor: pointer;
            transition: background 0.3s ease;
            width: 100%;
        }

        button:hover {
            background-color: #9c27b0;
        }

        #result {
            min-height: 1.5rem;
            font-weight: bold;
            margin-top: 0.5rem;
            color: #f3e5f5;
            text-shadow: 1px 1px #12005e;
        }

        a {
            display: inline-block;
            margin-top: 1.5rem;
            padding: 0.5rem 1rem;
            background-color: #ba68c8;
            color: #4a148c;
            text-decoration: none;
            border-radius: 8px;
            font-weight: bold;
            transition: background 0.3s ease;
        }

        a:hover {
            background-color: #ab47bc;
        }

        .emoji {
            font-size: 2.5rem;
            margin-top: 1.5rem;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>Delete a File</h1>
        <form id="deleteForm">
            <input
                type="text"
                id="filename"
                name="filename"
                placeholder="Enter filename to delete"
                required
                autocomplete="off"
            />
            <button type="submit">Delete File</button>
        </form>
        <div id="result"></div>
        <a href="/">← Return Home</a>
        <div class="emoji">📁🗑️💜</div>
    </div>

    <script>
        document.getElementById('deleteForm').addEventListener('submit', function(e) {
            e.preventDefault();
            const filename = document.getElementById('filename').value.trim();

            if (filename.length === 0) {
                alert('Please enter a filename');
                return;
            }

            if (filename.includes('/') || filename.includes('\\')) {
                alert('Invalid filename: slashes are not allowed');
                return;
            }

            fetch('/delete-upload', {
                method: 'DELETE',
                headers: {
                    'X-Filename': filename
                }
            })
            .then(response => {
                if (!response.ok) {
                    // Handle different HTTP errors explicitly
                    if (response.status === 404) {
                        throw new Error('File not found');
                    } else if (response.status === 400) {
                        throw new Error('Bad request');
                    } else if (response.status === 500) {
                        throw new Error('Server error');
                    } else {
                        throw new Error('Unexpected error: ' + response.status);
                    }
                }
                return response.text();
            })
            .then(text => {
                document.getElementById('result').textContent = text;
            })
            .catch(err => {
                // Show the error message nicely on the page instead of console only
                document.getElementById('result').textContent = err.message;
                console.error(err);
            });
        });
    </script>
</body>
</html>
EOF






# create cgi-tester.html
cat << 'EOF' > ./www/cgi-tester.html
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>CGI Script Tester</title>
    <style>
        @keyframes float {
            0% { transform: translateY(0); }
            50% { transform: translateY(-8px); }
            100% { transform: translateY(0); }
        }

        body {
            margin: 0;
            padding: 0;
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #0a0f2c, #1c2e4a);
            color: #dfe6f3;
            display: flex;
            align-items: center;
            justify-content: center;
            flex-direction: column;
            height: 100vh;
            overflow: hidden;
        }

        .container {
            background-color: rgba(10, 15, 44, 0.95);
            padding: 3rem;
            border-radius: 16px;
            box-shadow: 0 8px 24px rgba(0,0,0,0.4);
            text-align: center;
            animation: float 3s ease-in-out infinite;
            max-width: 700px;
            width: 90%;
        }

        h1 {
            font-size: 3rem;
            margin-bottom: 1rem;
            color: #a6bdfc;
            text-shadow: 1px 1px #000814;
        }

        p {
            font-size: 1.2rem;
            color: #a0c4ff;
        }

        button {
            padding: 0.75rem 1.5rem;
            background-color: #3f51b5;
            color: #ffffff;
            border: none;
            border-radius: 10px;
            font-weight: bold;
            font-size: 1rem;
            cursor: pointer;
            transition: background 0.3s ease;
            margin: 0.5rem;
        }

        button:hover {
            background-color: #303f9f;
        }

        #output {
            margin-top: 2rem;
            background-color: #e3e8f7;
            color: #0a0f2c;
            padding: 1rem;
            border-radius: 12px;
            font-family: monospace;
            white-space: pre-wrap;
            text-align: left;
            box-shadow: 0 4px 16px rgba(0,0,0,0.2);
        }

        a {
            display: inline-block;
            margin-top: 2rem;
            padding: 0.5rem 1rem;
            background-color: #5c6bc0;
            color: #ffffff;
            text-decoration: none;
            border-radius: 8px;
            font-weight: bold;
            transition: background 0.3s ease;
        }

        a:hover {
            background-color: #3949ab;
        }

        .emoji {
            font-size: 2rem;
            margin-top: 1rem;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>🧪 CGI Script Tester</h1>
        <p>Click a button to test a CGI script:</p>

        <button onclick="runCGI('http://127.0.0.1:8088/cgi-py')">Run hello.py (GET)</button>
        <button onclick="runCGIPost('http://127.0.0.1:8088/cgi-py')">hello.py (POST)</button>
        <button onclick="runCGI('http://127.0.0.1:8088/cgi-rb')">RUBY script (GET)</button>
        <button onclick="runCGIPost('http://127.0.0.1:8088/cgi-rb')">405 hello.rb (POST)</button>

        <div id="output">Script output will appear here...</div>

        <a href="http://127.0.0.1:8088">← Return Home</a>
        <div class="emoji">🛰️⚙️🌌</div>
    </div>

    <script>
        function runCGI(url) {
            fetch(url)
                .then(res => {
                    if (!res.ok) {
                        // If status is not 200, show the status code instead of the body
                        document.getElementById('output').textContent = "Error " + res.status + ": " + res.statusText;
                        return;
                    }
                    return res.text().then(text => {
                        document.getElementById('output').textContent = text;
                    });
                })
                .catch(err => {
                    document.getElementById('output').textContent = "Fetch error: " + err;
                });
        }

        function runCGIPost(url) {
            fetch(url, {
                method: 'POST',
                headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
                body: 'name=Webserv&test=123'
            })
            .then(res => {
                if (!res.ok) {
                    document.getElementById('output').textContent = "Error " + res.status + ": " + res.statusText;
                    return;
                }
                return res.text().then(text => {
                    document.getElementById('output').textContent = text;
                });
            })
            .catch(err => {
                document.getElementById('output').textContent = "Fetch error: " + err;
            });
        }
    </script>
</body>
</html>