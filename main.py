from flask import Flask, render_template
from Parser import *

app = Flask(__name__)
app.config['DEBUG'] = True
@app.route("/")
def index():
    return render_template("index.html", data=get_process())

@app.route("/process")
def get_process():
	data = refreshProcesses();
	data_formatted = []
	for process in data:
		data_formatted.append({"name": process.name, "pid": process.pid, "ppid": process.ppid, "threads": len(process.threads)})
	return data_formatted
if __name__ == "__main__":
    app.run()