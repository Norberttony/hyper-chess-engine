
const spawn = require("child_process").spawn;

class EngineProcess {
    constructor(engine, handler){
        this.engine = engine;
        this.handler = handler;
        this.hasStarted = false;
        this.proc;
    }

    start(){
        if (this.hasStarted)
            this.stop();

        this.hasStarted = true;

        this.proc = spawn(this.engine.path);

        const engine = this;
        this.proc.stdout.on("data", (data => {
            engine.handler(engine.engine, data.toString());
        }));
    }

    stop(){
        this.proc.kill();
        this.hasStarted = false;
    }
    
    write(cmd){
        if (this.hasStarted){
            this.proc.stdin.write(`${cmd}\n`);
        }
    }
}

module.exports = { EngineProcess };
