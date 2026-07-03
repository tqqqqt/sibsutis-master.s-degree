import os

# Оптимизация потоков ONNX Runtime под 6-ядерный Ryzen
os.environ["OMP_NUM_THREADS"] = "6"
os.environ["ONNX_NUM_THREADS"] = "6"
os.environ["MKL_NUM_THREADS"] = "6"

import tkinter as tk
from tkinter import font
import cv2
import numpy as np
from PIL import Image, ImageTk
import insightface
import pickle, os as _os


class FaceApp:
    # Catppuccin Mocha
    BASE = "#1e1e2e"
    MANTLE = "#181825"
    SURFACE0 = "#313244"
    SURFACE1 = "#45475a"
    TEXT = "#cdd6f4"
    SUBTEXT = "#a6adc8"
    BLUE = "#89b4fa"
    GREEN = "#a6e3a1"
    RED = "#f38ba8"
    YELLOW = "#f9e2af"

    BGR_GREEN = (161, 227, 166)
    BGR_RED = (168, 139, 243)
    BGR_YELLOW = (175, 226, 249)

    DB_FILE = "faces.pkl"
    THRESHOLD = 0.35
    REG_FRAMES = 10
    INFERENCE_SKIP = 3  # Запускать модель каждый N-й кадр

    def __init__(self, root):
        self.root = root
        self.root.title("Верификация лиц 1:1")
        self.root.geometry("700x640")
        self.root.configure(bg=self.BASE)
        self.root.resizable(False, False)

        self._setup_font()

        self.mode = "idle"
        self.threshold = self.THRESHOLD
        self.db = self.load_db()
        self.reg_embs = []
        self.register_name = ""
        self.is_running = False
        self.model = None
        self.cap = None
        self._dialog_open = False

        # Оптимизация цикла
        self.frame_idx = 0
        self.cached_faces = []

        self._build_ui()
        self.root.after(100, self._init_backend)

    def _setup_font(self):
        families = font.families()
        candidates = [
            "FiraCode Nerd Font",
            "FiraCode Nerd Font Mono",
            "FiraCode",
            "Fira Code",
            "Noto Sans Mono",
            "monospace",
        ]
        chosen = next((c for c in candidates if c in families), "monospace")
        self.FONT = (chosen, 10)
        for name in ("TkDefaultFont", "TkFixedFont", "TkTextFont", "TkMenuFont"):
            try:
                font.nametofont(name).configure(family=chosen, size=10)
            except tk.TclError:
                pass

    def _style_btn(self, w):
        w.config(
            bg=self.SURFACE0,
            fg=self.TEXT,
            activebackground=self.SURFACE1,
            activeforeground=self.BLUE,
            highlightthickness=0,
            bd=0,
            relief="flat",
            font=self.FONT,
            cursor="hand2",
        )

    # ──────────────────────────────────────────────────────────────
    # Кастомные модальные окна (Catppuccin + FiraCode)
    # ──────────────────────────────────────────────────────────────
    def _dialog_base(self, title, width=360, height=150):
        dlg = tk.Toplevel(self.root)
        dlg.title(title)
        dlg.geometry(f"{width}x{height}")
        dlg.transient(self.root)
        dlg.configure(bg=self.BASE)
        dlg.resizable(False, False)
        dlg.protocol("WM_DELETE_WINDOW", lambda: None)
        x = self.root.winfo_x() + (self.root.winfo_width() - width) // 2
        y = self.root.winfo_y() + (self.root.winfo_height() - height) // 2
        dlg.geometry(f"+{x}+{y}")
        return dlg

    def _ask_name_dialog(self):
        dlg = self._dialog_base("Регистрация", width=340, height=140)
        tk.Label(
            dlg, text="Введите имя:", bg=self.BASE, fg=self.TEXT, font=self.FONT
        ).pack(pady=(18, 6))
        var = tk.StringVar()
        entry = tk.Entry(
            dlg,
            textvariable=var,
            bg=self.SURFACE0,
            fg=self.TEXT,
            font=self.FONT,
            insertbackground=self.TEXT,
            selectbackground=self.BLUE,
            selectforeground=self.BASE,
            highlightthickness=0,
            bd=0,
            relief="flat",
        )
        entry.pack(fill="x", padx=20, pady=5)
        entry.focus_set()

        result = [None]

        def _ok():
            result[0] = var.get().strip()
            dlg.destroy()

        def _cancel():
            dlg.destroy()

        btn_f = tk.Frame(dlg, bg=self.BASE)
        btn_f.pack(fill="x", pady=12)
        b1 = tk.Button(btn_f, text="OK", command=_ok)
        b2 = tk.Button(btn_f, text="Отмена", command=_cancel)
        self._style_btn(b1)
        self._style_btn(b2)
        b1.pack(side="left", expand=True, padx=10)
        b2.pack(side="right", expand=True, padx=10)

        dlg.protocol("WM_DELETE_WINDOW", _cancel)
        entry.bind("<Return>", lambda e: _ok())
        dlg.bind("<Escape>", lambda e: _cancel())
        dlg.grab_set()
        self.root.wait_window(dlg)
        return result[0]

    def _ask_yesno_dialog(self, message):
        dlg = self._dialog_base("Подтверждение", width=380, height=130)
        tk.Label(
            dlg,
            text=message,
            bg=self.BASE,
            fg=self.TEXT,
            font=self.FONT,
            wraplength=320,
            justify="center",
        ).pack(pady=20)

        result = [False]

        def _yes():
            result[0] = True
            dlg.destroy()

        def _no():
            dlg.destroy()

        btn_f = tk.Frame(dlg, bg=self.BASE)
        btn_f.pack(fill="x", pady=10)
        b1 = tk.Button(btn_f, text="Да", command=_yes)
        b2 = tk.Button(btn_f, text="Нет", command=_no)
        self._style_btn(b1)
        self._style_btn(b2)
        b1.pack(side="left", expand=True, padx=10)
        b2.pack(side="right", expand=True, padx=10)

        dlg.protocol("WM_DELETE_WINDOW", _no)
        dlg.bind("<Escape>", lambda e: _no())
        dlg.bind("<Return>", lambda e: _yes())
        dlg.grab_set()
        self.root.wait_window(dlg)
        return result[0]

    def _show_message_dialog(self, message, level="info"):
        dlg = self._dialog_base("Системное сообщение", width=360, height=140)
        fg = self.RED if level == "error" else self.TEXT
        tk.Label(
            dlg,
            text=message,
            bg=self.BASE,
            fg=fg,
            font=self.FONT,
            wraplength=300,
            justify="center",
        ).pack(pady=18)

        def _ok():
            dlg.destroy()

        b = tk.Button(dlg, text="OK", command=_ok)
        self._style_btn(b)
        b.pack(pady=10)
        b.focus_set()

        dlg.protocol("WM_DELETE_WINDOW", _ok)
        dlg.bind("<Return>", lambda e: _ok())
        dlg.bind("<Escape>", lambda e: _ok())
        dlg.grab_set()
        self.root.wait_window(dlg)

    # ──────────────────────────────────────────────────────────────
    # Основной интерфейс
    # ──────────────────────────────────────────────────────────────
    def _build_ui(self):
        self.status = tk.Label(
            self.root,
            text="[INIT] Загрузка модели...",
            fg=self.BLUE,
            bg=self.BASE,
            anchor="w",
            font=self.FONT,
        )
        self.status.pack(fill="x", padx=14, pady=(8, 4))

        self.vid_frame = tk.Frame(
            self.root,
            width=660,
            height=380,
            bg=self.MANTLE,
            highlightbackground=self.SURFACE1,
            highlightthickness=1,
        )
        self.vid_frame.pack(padx=14, pady=4)
        self.vid_frame.pack_propagate(False)

        self.video = tk.Label(
            self.vid_frame,
            bg=self.MANTLE,
            fg=self.SUBTEXT,
            text="[ОЖИДАНИЕ КАМЕРЫ]",
            font=self.FONT,
        )
        self.video.pack(fill="both", expand=True)

        ctrl = tk.Frame(self.root, bg=self.BASE, bd=0)
        ctrl.pack(fill="x", padx=14, pady=2)

        self.btn_reg = tk.Button(ctrl, text="Регистрация", command=self._start_register)
        self._style_btn(self.btn_reg)
        self.btn_reg.pack(side="left", padx=3, ipadx=10, ipady=4)

        self.btn_ver = tk.Button(ctrl, text="Распознать", command=self._start_verify)
        self._style_btn(self.btn_ver)
        self.btn_ver.pack(side="left", padx=3, ipadx=10, ipady=4)

        self.btn_stop = tk.Button(ctrl, text="Стоп", command=self._stop)
        self._style_btn(self.btn_stop)
        self.btn_stop.pack(side="left", padx=3, ipadx=10, ipady=4)

        thr = tk.Frame(self.root, bg=self.BASE, bd=0)
        thr.pack(fill="x", padx=14, pady=4)

        tk.Label(
            thr, text="порог:", fg=self.SUBTEXT, bg=self.BASE, font=self.FONT
        ).pack(side="left")
        tk.Label(thr, text="0.20", fg=self.SUBTEXT, bg=self.BASE, font=self.FONT).pack(
            side="left", padx=(4, 0)
        )

        self.thr_var = tk.DoubleVar(value=self.threshold)
        self.slider = tk.Scale(
            thr,
            from_=0.20,
            to_=0.55,
            resolution=0.01,
            orient="horizontal",
            variable=self.thr_var,
            showvalue=0,
            command=lambda v: self.thr_lbl.config(text=f"{float(v):.2f}"),
            bg=self.BASE,
            fg=self.TEXT,
            troughcolor=self.SURFACE1,
            activebackground=self.BLUE,
            highlightthickness=0,
            bd=0,
            font=self.FONT,
        )
        self.slider.pack(side="left", fill="x", expand=True, padx=8)

        tk.Label(thr, text="0.55", fg=self.SUBTEXT, bg=self.BASE, font=self.FONT).pack(
            side="left", padx=(0, 4)
        )

        self.thr_lbl = tk.Label(
            thr,
            text=f"{self.threshold:.2f}",
            fg=self.TEXT,
            bg=self.BASE,
            font=self.FONT,
            width=5,
            anchor="e",
        )
        self.thr_lbl.pack(side="right", padx=4)

        db_frame = tk.LabelFrame(
            self.root,
            text=" зарегистрированные лица ",
            fg=self.SUBTEXT,
            bg=self.BASE,
            highlightbackground=self.SURFACE1,
            highlightthickness=1,
            font=self.FONT,
        )
        db_frame.pack(fill="both", expand=True, padx=14, pady=8)

        self.listbox = tk.Listbox(
            db_frame,
            bg=self.MANTLE,
            fg=self.TEXT,
            selectbackground=self.BLUE,
            selectforeground=self.BASE,
            highlightthickness=0,
            bd=0,
            font=self.FONT,
        )
        self.listbox.pack(side="left", fill="both", expand=True)
        self._update_listbox()

        btns = tk.Frame(db_frame, bg=self.BASE, bd=0)
        btns.pack(side="right", fill="y", padx=6, pady=4)

        self.btn_rem = tk.Button(btns, text="Удалить", command=self._remove_selected)
        self._style_btn(self.btn_rem)
        self.btn_rem.pack(fill="x", pady=3)

        self.btn_clr = tk.Button(btns, text="Очистить", command=self._clear_db)
        self._style_btn(self.btn_clr)
        self.btn_clr.pack(fill="x", pady=3)

    def _init_backend(self):
        self.status.config(text="[LOAD] Инициализация antelopev2...", fg=self.SUBTEXT)
        self.root.update()
        try:
            # det_size=256 снижает нагрузку на CPU без потери точности для вебкамеры
            self.model = insightface.app.FaceAnalysis(
                name="antelopev2", providers=["CPUExecutionProvider"]
            )
            self.model.prepare(ctx_id=0, det_size=(256, 256))
        except Exception as e:
            self.status.config(text="[ERROR] Ошибка загрузки модели", fg=self.RED)
            self._show_message_dialog(f"Не удалось загрузить модель.\n{e}", "error")
            return

        self.cap = cv2.VideoCapture(0)
        if not self.cap.isOpened():
            self.status.config(text="[ERROR] Камера недоступна", fg=self.RED)
            self._show_message_dialog("Не удалось открыть камеру.", "error")
            return

        # Оптимизация V4L2 для Arch Linux
        self.cap.set(cv2.CAP_PROP_FOURCC, cv2.VideoWriter_fourcc("M", "J", "P", "G"))
        self.cap.set(cv2.CAP_PROP_BUFFERSIZE, 1)
        self.cap.set(cv2.CAP_PROP_FRAME_WIDTH, 640)
        self.cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)
        self.cap.set(cv2.CAP_PROP_FPS, 30)
        for _ in range(5):
            self.cap.grab()

        self.status.config(text="[READY] Система готова", fg=self.GREEN)
        self.is_running = True
        self._loop()

    def _loop(self):
        if not self.is_running:
            self.root.after(40, self._loop)
            return

        try:
            ret, frame = self.cap.read()
            if not ret or frame is None:
                self.root.after(40, self._loop)
                return

            out = frame.copy()

            # Инференс только каждый 3-й кадр (экономит ~60% CPU)
            self.frame_idx += 1
            if self.frame_idx % self.INFERENCE_SKIP == 0:
                self.cached_faces = self.model.get(frame)
            faces = self.cached_faces

            if self.mode == "register":
                if faces and len(self.reg_embs) < self.REG_FRAMES:
                    self.reg_embs.append(faces[0].normed_embedding)
                    cnt = len(self.reg_embs)
                    cv2.putText(
                        out,
                        f"collecting: {cnt}/{self.REG_FRAMES}",
                        (20, 40),
                        cv2.FONT_HERSHEY_SIMPLEX,
                        0.8,
                        self.BGR_GREEN,
                        2,
                    )
                    if cnt >= self.REG_FRAMES:
                        self._finish_register()
                else:
                    cv2.putText(
                        out,
                        "position face in frame",
                        (20, 40),
                        cv2.FONT_HERSHEY_SIMPLEX,
                        0.8,
                        self.BGR_YELLOW,
                        2,
                    )

            elif self.mode == "verify" and faces:
                emb = faces[0].normed_embedding
                bbox = faces[0].bbox.astype(int)
                best_sim, best_name = -1.0, "unknown"
                for name, ref in self.db.items():
                    sim = float(np.dot(emb, ref))
                    if sim > best_sim:
                        best_sim, best_name = sim, name

                is_match = best_sim >= self.threshold
                color = self.BGR_GREEN if is_match else self.BGR_RED
                label = f"MATCH: {best_name}" if is_match else "UNKNOWN"
                cv2.rectangle(out, (bbox[0], bbox[1]), (bbox[2], bbox[3]), color, 2)
                cv2.putText(
                    out,
                    f"{label} ({best_sim:.2f})",
                    (bbox[0], bbox[1] - 10),
                    cv2.FONT_HERSHEY_SIMPLEX,
                    0.6,
                    color,
                    2,
                )

            # Быстрый ресайз через cv2 вместо PIL
            h, w = out.shape[:2]
            scale = min(660 / w, 380 / h)
            new_w, new_h = int(w * scale), int(h * scale)
            if new_w != w or new_h != h:
                out = cv2.resize(out, (new_w, new_h), interpolation=cv2.INTER_LINEAR)

            rgb = cv2.cvtColor(out, cv2.COLOR_BGR2RGB)
            photo = ImageTk.PhotoImage(image=Image.fromarray(rgb))
            self.video.config(image=photo, text="")
            self.video.image = photo

        except Exception as e:
            print(f"[WARN] frame skip: {e}")

        self.root.after(35, self._loop)

    def _start_register(self):
        if self._dialog_open:
            return
        if self.mode != "idle":
            self._stop()
            self.root.after(100, self._ask_register_name)
        else:
            self._ask_register_name()

    def _ask_register_name(self):
        self._dialog_open = True
        name = self._ask_name_dialog()
        self._dialog_open = False
        if not name:
            return

        self.register_name = name
        self.reg_embs = []
        self.mode = "register"
        self.status.config(
            text=f"[REGISTER] Сбор данных: {self.register_name}", fg=self.YELLOW
        )

    def _finish_register(self):
        if not self.reg_embs:
            self.mode = "idle"
            self.register_name = ""
            return

        avg = np.mean(self.reg_embs, axis=0)
        self.db[self.register_name] = avg
        self.save_db()
        self._update_listbox()

        self.mode = "idle"
        self.register_name = ""
        self.reg_embs = []
        self.status.config(text="[OK] Регистрация завершена", fg=self.GREEN)

    def _start_verify(self):
        if self.mode != "idle" or not self.db:
            return
        self.mode = "verify"
        self.status.config(text="[VERIFY] Распознавание активно", fg=self.BLUE)

    def _stop(self):
        self.mode = "idle"
        self.register_name = ""
        self.reg_embs = []
        self.status.config(text="[STOP] Ожидание", fg=self.SUBTEXT)

    def _update_listbox(self):
        self.listbox.delete(0, "end")
        for n in sorted(self.db.keys()):
            self.listbox.insert("end", n)

    def _remove_selected(self):
        sel = self.listbox.curselection()
        if not sel:
            return
        name = self.listbox.get(sel[0])
        if self._ask_yesno_dialog(f"Удалить '{name}' из базы?"):
            del self.db[name]
            self.save_db()
            self._update_listbox()

    def _clear_db(self):
        if self._ask_yesno_dialog("Удалить все зарегистрированные лица?"):
            self.db = {}
            if _os.path.exists(self.DB_FILE):
                _os.remove(self.DB_FILE)
            self._update_listbox()
            self.status.config(text="[DB] База очищена", fg=self.SUBTEXT)

    def save_db(self):
        with open(self.DB_FILE, "wb") as f:
            pickle.dump(self.db, f)

    def load_db(self):
        if _os.path.exists(self.DB_FILE):
            with open(self.DB_FILE, "rb") as f:
                return pickle.load(f)
        return {}

    def _cleanup(self):
        self.is_running = False
        if self.cap:
            self.cap.release()
        cv2.destroyAllWindows()


if __name__ == "__main__":
    root = tk.Tk()
    root.option_add("*Ttk::background", FaceApp.BASE)
    app = FaceApp(root)
    root.protocol("WM_DELETE_WINDOW", lambda: (app._cleanup(), root.destroy()))
    root.mainloop()
