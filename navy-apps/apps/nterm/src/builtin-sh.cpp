#include <nterm.h>
#include <stdarg.h>
#include <unistd.h>
#include <SDL.h>

char handle_key(SDL_Event *ev);

static void sh_printf(const char *format, ...) {
  static char buf[256] = {};
  va_list ap;
  va_start(ap, format);
  int len = vsnprintf(buf, 256, format, ap);
  va_end(ap);
  term->write(buf, len);
}

static void sh_banner() {
  sh_printf("Built-in Shell in NTerm (NJU Terminal)\n\n");
}

static void sh_prompt() {
  sh_printf("sh> ");
}

static void sh_handle_cmd(const char *cmd) {
	char buf[24];
	strcpy(buf, cmd);
	for(int i = 0; i < 24; i++)
		if(buf[i] == '\n'){
		  buf[i] = '\0';
			break;
		}
	if(strncmp(buf, "PATH=", 5) == 0){
		setenv("PATH", buf + 5, 1);
		return;
	}
	char *argv[] = {buf, 0};
	//execve(buf, argv, NULL);
	execvp(buf, argv);
	sh_printf("%s: file not found\n", buf);
}

void builtin_sh_run() {
  sh_banner();
  sh_prompt();

  while (1) {
    SDL_Event ev;
    if (SDL_PollEvent(&ev)) {
      if (ev.type == SDL_KEYUP || ev.type == SDL_KEYDOWN) {
        const char *res = term->keypress(handle_key(&ev));
        if (res) {
          sh_handle_cmd(res);
          sh_prompt();
        }
      }
    }
    refresh_terminal();
  }
}
