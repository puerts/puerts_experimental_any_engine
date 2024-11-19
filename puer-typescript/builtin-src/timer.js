; (function () {
    const g = typeof globalThis == 'undefined' ? this : globalThis;
    
    class PriorityQueue {
        constructor(data = [], compare = (a, b) => a - b) {
            this.data = data;
            this.length = this.data.length;
            this.compare = compare;
            if (this.length > 0) {
                for (let i = (this.length >> 1) - 1; i >= 0; i--)
                    this._down(i);
            }
        }
        push(item) {
            this.data.push(item);
            this.length++;
            this._up(this.length - 1);
        }
        pop() {
            if (this.length === 0)
                return undefined;
            const top = this.data[0];
            const bottom = this.data.pop();
            this.length--;
            if (this.length > 0) {
                this.data[0] = bottom;
                this._down(0);
            }
            return top;
        }
        peek() {
            return this.data[0];
        }
        _up(pos) {
            const { data, compare } = this;
            const item = data[pos];
            while (pos > 0) {
                const parent = (pos - 1) >> 1;
                const current = data[parent];
                if (compare(item, current) >= 0)
                    break;
                data[pos] = current;
                pos = parent;
            }
            data[pos] = item;
        }
        _down(pos) {
            const { data, compare } = this;
            const halfLength = this.length >> 1;
            const item = data[pos];
            while (pos < halfLength) {
                let left = (pos << 1) + 1;
                let best = data[left];
                const right = left + 1;
                if (right < this.length && compare(data[right], best) < 0) {
                    left = right;
                    best = data[right];
                }
                if (compare(best, item) >= 0)
                    break;
                data[pos] = best;
                pos = left;
            }
            data[pos] = item;
        }
    }
    const removing_timers = new Set();
    const timers = new PriorityQueue([], (a, b) => a.next_time - b.next_time);
    let next = 0;

    g.timerUpdate = function timerUpdate() {
        let now = null;
        while (true) {
            const time = timers.peek();
            if (!time) {
                break;
            }
            if (!now) {
                now = Date.now();
            }
            if (time.next_time <= now) {
                timers.pop();
                if (removing_timers.has(time.id)) {
                    removing_timers.delete(time.id);
                }
                else {
                    if (time.timeout) {
                        time.next_time = now + time.timeout;
                        timers.push(time);
                    }
                    time.handler(...time.args);
                }
            }
            else {
                break;
            }
        }
    };
    g.setTimeout = (fn, time, ...arg) => {
        if (typeof fn !== 'function') {
            throw new Error(`Callback must be a function. Received ${typeof fn}`);
        }
        let t = 0;
        if (time > 0)
            t = time;
        timers.push({
            id: ++next,
            next_time: t + Date.now(),
            args: arg,
            handler: fn,
        });
        return next;
    };
    g.setInterval = (fn, time, ...arg) => {
        if (typeof fn !== 'function') {
            throw new Error(`Callback must be a function. Received ${typeof fn}`);
        }
        let t = 10;
        if (time != null && time > 10)
            t = time;
        timers.push({
            id: ++next,
            next_time: t + Date.now(),
            handler: fn,
            args: arg,
            timeout: t
        });
        return next;
    };
    g.clearInterval = (id) => {
        removing_timers.add(id);
    };
    g.clearTimeout = globalThis.clearInterval;
})()
