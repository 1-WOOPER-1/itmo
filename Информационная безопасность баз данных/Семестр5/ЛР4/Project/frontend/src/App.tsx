import { BrowserRouter, Routes, Route } from "react-router";
import { Login } from "./pages/login";
import { Main } from "./pages/main";

function App() {
  return (
    <BrowserRouter>
      <Routes>
        <Route path="/login" element={<Login />} />
        <Route path="/" element={<Main />} />
      </Routes>
    </BrowserRouter>
  );
}

export default App;
