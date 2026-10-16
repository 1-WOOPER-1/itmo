import { useState } from "react";
import { LogIn } from "lucide-react";
import { useNavigate } from "react-router";

const API = import.meta.env.VITE_API_BASE_URL;

export function Login() {
  const navigate = useNavigate();
  const [username, setUsername] = useState("");
  const [password, setPassword] = useState("");
  const [error, setError] = useState<string | null>(null);

  async function login(e: React.FormEvent<HTMLFormElement>) {
    e.preventDefault();

    const body = new URLSearchParams({ username, password });
    const res = await fetch(`${API}/login`, {
      method: "POST",
      headers: { "Content-Type": "application/x-www-form-urlencoded" },
      body,
    });
    if (!res.ok) {
      setError((await res.json()).detail);
      return;
    }
    const data = await res.json();
    localStorage.setItem("token", data.access_token);
    navigate("/");
  }

  return (
    <main className="flex justify-center items-center h-screen">
      <form className="flex flex-col gap-4" onSubmit={(e) => login(e)}>
        <h1>Войти в систему</h1>
        <input
          className={`p-2 rounded-md border border-border focus:outline-none focus:ring-2 focus:ring-primary ${error ? "border-error" : ""}`}
          type="username"
          placeholder="Имя пользователя"
          value={username}
          onChange={(e) => {
            setError(null);
            setUsername(e.target.value);
          }}
        />
        <input
          className={`p-2 rounded-md border border-border focus:outline-none focus:ring-2 focus:ring-primary ${error ? "border-error" : ""}`}
          type="password"
          placeholder="Пароль"
          value={password}
          onChange={(e) => {
            setError(null);
            setPassword(e.target.value);
          }}
        />
        {error && <p className="text-error">{error}</p>}
        <button
          className="flex items-center gap-2 bg-primary text-on-primary px-3 py-1.5 rounded-md hover:bg-primary/80 transition-colors duration-200"
          type="submit"
        >
          <LogIn className="w-4 h-4" />
          Login
        </button>
      </form>
    </main>
  );
}
