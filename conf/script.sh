# script to create the html files needed

#webserv/
#├── www/             # Static files go here
#│   └── index.html
#├── uploads/         # File uploads will be stored here
#├── cgi-bin/         # CGI scripts (e.g., .py, .php) go here
#│   └── hello.py
#└── server.conf      # Your configuration fil

# at root
rm -rf ./www
rm -rf ./uploads
rm -rf ./cgi-bin

mkdir -p ./www
mkdir -p ./www/errors
mkdir -p ./www/api
mkdir -p ./uploads
mkdir -p ./cgi-bin

chmod -R 755 ./

# default python script for CGI
cat << 'EOF' > ./cgi-bin/hello.py
#!/usr/bin/python3
import os

print("Content-Type: text/html\n")
print("<html><body>")
print("<h1>Hello from CGI!</h1>")
print(f"<p>PATH_INFO: {os.environ.get('PATH_INFO')}</p>")
print("</body></html>")
EOF

# chmod +x ./cgi-bin/hello.py



# create index.html
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
  # just for a 403 req
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
    <h4>Jsaintho et Gchauvot presents: </h4>
    <div class="card">
        <h1>42 WEBSERVS GOES BRRRRRRRRR!</h1>
        <p>Cette page is served by your <strong>webserv</strong> C++98 HTTP server.</p>
        <div class="links">
            <a href="/non-exzeistent">trigger le 404</a>
            <a href="/null"> 403 no autoindex</a>
            <a href="/files">trigger 403 avec autoindex (list les files)</a>
            <a href="/upload">upload endpoint</a>
            <a href="/cgi">CGI Endpoint</a>
            <form action="/files" method="POST" onsubmit="return confirm('trigger 405 error with POST?');" style="display:inline;">
                <button type="submit" style="background:none; border:none; color:blue; text-decoration:underline; cursor:pointer; padding:0;">
                    trigger le 405 (POST not allowed)
                </button>
            </form>
        </div>
    </div>
    <h5>all rights reserved. 
        <a href="https://github.com/youradrien">le j</a>
        <a href="https://github.com/gauchau> le g</a>
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
            50% { transform: translateY(-10px); }
            100% { transform: translateY(0); }
        }

        body {
            margin: 0;
            padding: 0;
            font-family: "Comic Sans MS", cursive, sans-serif;
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
        <h1>404</h1>
        <h4>
        the server cannot find the requested resource.
        in the browser, this means the URL is not recognized. in an API this can also mean that the endpoint is valid but the resource itself does not exist
        this response code is probably the most well known due to its frequent occurrence on the web.
        </h4>
        <p>Oops! you went into cyberspace cuh 🌌</p>
        <a href="/">retour à la maison</a>
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




# create index.html
cat << 'EOF' > ./www/special.html
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>Something Special</title>
  <style>
    html, body {
      margin: 0;
      padding: 0;
      height: 100%;
      overflow: hidden;
      font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
      background: radial-gradient(ellipse at center, #000010 0%, #000000 100%);
      color: white;
    }

    canvas {
      position: absolute;
      top: 0;
      left: 0;
      z-index: 0;
    }

    .content {
      position: relative;
      z-index: 1;
      height: 100%;
      display: flex;
      align-items: center;
      justify-content: center;
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
  </div>

  <script>
    // Starfield background
    const canvas = document.getElementById('stars');
    const ctx = canvas.getContext('2d');
    let stars = [];

    function dstars() {
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

    d-stars();
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

# create uploadz.html
cat << 'EOF' > ./www/uploadz.html
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
        <form action="/upload" method="post" enctype="multipart/form-data" autocomplete="off">
            <label for="fileUpload">Pick a file 🌸</label>
            <input type="file" id="fileUpload" name="file" />
            <br />
            <button type="submit">Upload</button>
        </form>
    </div>
</body>
</html>
EOF
