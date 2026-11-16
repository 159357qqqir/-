from flask import Flask, send_from_directory, render_template_string, os, request

app = Flask(__name__)
BASE_PATH = r"D:/"
# 安卓热点固定IP
ALLOW_IP = {"192.168.43.1"}

HTML_TPL = '''
<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8">
<title>D盘文件服务</title>
<style>
body{font-size:18px;}
a{text-decoration:none;}
li{margin:4px 0;}
</style>
</head>
<body>
<h2>D盘文件服务</h2>
<a href="../">⬅ 返回上级</a>
<ul>
{% for item in items %}
<li>
{% if item.is_dir %}
📁 <a href="{{ url_for('browse', subpath=item.name) }}">{{ item.name }}</a>
{% else %}
📄 <a href="{{ url_for('download', subpath=item.name) }}">{{ item.name }}</a>
{% endif %}
</li>
{% endfor %}
</ul>
</body>
</html>
'''

@app.before_request
def check_ip():
    client_ip = request.remote_addr
    if client_ip not in ALLOW_IP:
        return "403 禁止访问", 403

@app.route('/', defaults={'subpath': ''})
@app.route('/browse/<path:subpath>')
def browse(subpath):
    current = os.path.join(BASE_PATH, subpath)
    current = os.path.abspath(current)
    base = os.path.abspath(BASE_PATH)
    if not current.startswith(base):
        return "403 禁止访问", 403
    if not os.path.exists(current):
        return "404 文件不存在", 404

    items = []
    for name in os.listdir(current):
        full_path = os.path.join(current, name)
        items.append({
            "name": name,
            "is_dir": os.path.isdir(full_path)
        })
    return render_template_string(HTML_TPL, items=items)


@app.route('/download/<path:subpath>')
def download(subpath):
    current = os.path.join(BASE_PATH, subpath)
    current = os.path.abspath(current)
    base = os.path.abspath(BASE_PATH)
    if not current.startswith(base):
        return "403 禁止访问", 403
    folder = os.path.dirname(current)
    filename = os.path.basename(current)
    return send_from_directory(folder, filename, as_attachment=True)


if __name__ == '__main__':
    app.run(host="0.0.0.0", port=8080, debug=False)
