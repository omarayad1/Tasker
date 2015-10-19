from flask import Flask, render_template,jsonify
from Parser import *
import time

app = Flask(__name__)
app.config['DEBUG'] = True

@app.route('/', defaults={'cpu_no': 0 })
@app.route("/<int:cpu_no>")
def index(cpu_no):
    return render_template(
    	"index.html",
    	processes=get_process(),
    	cpus=get_cpus(output=1)[cpu_no],
    	length=len(get_cpus(output=1)),
    	curr=cpu_no
    	)

@app.route("/process")
def get_process():
	data = refreshProcesses();
	data_formatted = []
	for process in data:
		data_formatted.append({
			"name": process.name,
			"pid": process.pid,
			"ppid": process.ppid,
			"threads": len(process.threads)
			})
	return data_formatted

@app.route('/process/<pid>/', defaults={'cpu_no': 0 })
@app.route("/process/<pid>/<int:cpu_no>")
def get_one_process(pid,cpu_no):
	z_one = {}
	data = refreshProcesses()
	cpu_data = refreshCPU()
	for el in data:
		if el.pid == pid:
			z_one = el
			return render_template("process.html", data = z_one, cpu_no = len(cpu_data),curr=cpu_no)

@app.route('/process/load/<pid>/', defaults={'cpu_no': 0 })
@app.route("/process/load/<pid>/<int:cpu_no>")
def get_one_process_load(pid,cpu_no):
	threads = []
	cpu_data = refreshCPU()
	proc_data = refreshProcesses()
	proc_data = ProcVector(proc_data)
	updateCPUData(CpuVector(cpu_data))
	updateProcessData(proc_data, CpuVector(cpu_data))
	time.sleep(1)
	updateCPUData(CpuVector(cpu_data))
	updateProcessData(proc_data, CpuVector(cpu_data))
	usage = 0
	for el in proc_data:
		if el.pid == pid:
			if cpu_no == 0:
				usage = sum([thread.usagePercentage for thread in el.threads])/4.0
			else:
				usage = sum([thread.usagePercentage for thread in el.threads if thread.cpu == cpu_no])
			
	return jsonify({"usage":usage*100000})

@app.route("/cpu")
def get_cpus(output=0):
	cpu_data = refreshCPU()
	data = updateCPUData(CpuVector(cpu_data))
	time.sleep(1)
	data = updateCPUData(CpuVector(cpu_data))
	items = []
	for item in data:
		items.append({"name": item.name, "total": int(item.total), "idle": int(item.idle), "usage": item.usage})
	if output:
		return items
	return jsonify({"data": items})

if __name__ == "__main__":
    app.run()