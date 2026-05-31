const fs = require("fs");

// -------------------------------------------------------------------------
//                       ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ
// -------------------------------------------------------------------------
/**
 * Вычисляет значение полинома в точке x схемой Горнера
 * @param {number[]} coeffs
 * @param {number} x
 */
function f(coeffs, x) {
  let res = 0;
  for (let i = 0; i < coeffs.length; i++) res = res * x + coeffs[i];
  return res;
}

/**
 * Вычисляет значение производной функции в точке x схемой Горнера
 * @param {number[]} coeffs
 * @param {number} x
 */
function df(coeffs, x) {
  let res = 0;
  let n = coeffs.length - 1;
  for (let i = 0; i < n; i++) res = res * x + coeffs[i] * (n - i);
  return res;
}

/**
 * Вычисляет значение второй производной функции в точке x схемой Горнера
 * @param {number[]} coeffs
 * @param {number} x
 */
function ddf(coeffs, x) {
  let res = 0;
  let n = coeffs.length - 1;
  for (let i = 0; i < n - 1; i++)
    res = res * x + coeffs[i] * (n - i) * (n - i - 1);
  return res;
}

/**
 * Вычисляет границы поиска корней по теореме Коши
 * @param {number[]} coeffs
 */
function cachyBound(coeffs) {
  let cleanedCoeffs = [];
  let zeroFlag = false;
  for (let i = 0; i < coeffs.length; i++) {
    if (coeffs[i] !== 0 || zeroFlag) {
      cleanedCoeffs.push(coeffs[i]);
      zeroFlag = true;
    }
  }

  if (cleanedCoeffs.length <= 1) return 100;

  const maxCoeff =
    cleanedCoeffs.length > 1
      ? Math.max(...cleanedCoeffs.slice(1).map(Math.abs))
      : 0;
  return 1 + maxCoeff / Math.abs(cleanedCoeffs[0]);
}

/**
 * Поиск интервала, где функция меняет свой знак
 * @param {number[]} coeffs
 * @param {number} bound
 */
function findInterval(coeffs, bound) {
  let step = bound / 20000;
  let l = -bound;
  while (l < bound) {
    let r = l + step;
    if (f(coeffs, l) * f(coeffs, r) < 0) return [l, r];
    l += step;
  }
  return [-bound / 2, bound / 2];
}

// -------------------------------------------------------------------------
//                       МЕТОДЫ ДЛЯ НАХОЖДЕНИЯ КОРНЕЙ
// -------------------------------------------------------------------------

/**
 * Схема Горнера для понижения степени полинома.
 * После деления на (x - root) уменьшает длину coeffs на единицу
 * @param {number[]} coeffs
 * @param {number} root
 */
function hornersShema(coeffs, root) {
  const k = coeffs.length;
  let newCoeffs = new Array(k - 1);
  newCoeffs[0] = coeffs[0];
  for (let i = 1; i < k - 1; i++) {
    newCoeffs[i] = coeffs[i] + newCoeffs[i - 1] * root;
  }
  return newCoeffs;
}

/**
 * Теорема Виета для полинома степени 2
 * @param {number[]} coeffs
 */
function vietasTheorem(coeffs) {
  const [a, b, c] = coeffs;
  const D = b * b - 4 * a * c;
  if (D >= 0)
    return [(-b + Math.sqrt(D)) / (2 * a), (-b - Math.sqrt(D)) / (2 * a)];
  if (D === 0) return [-b / (2 * a)];
  else return null;
}

/**
 * Метод Кардано для полинома степени 3
 * @param {number[]} coeffs
 */
function cardanosMethod(coeffs) {
  const [a, b, c, d] = coeffs;

  let A = b / a;
  let B = c / a;
  let C = d / a;

  const p = B - A ** 2 / 3;
  const q = (2 * A ** 3) / 27 - (A * B) / 3 + C;
  const Q = (p / 3) ** 3 + (q / 2) ** 2;
  const shift = A / 3;

  const cbrt = (x) => Math.sign(x) * Math.pow(Math.abs(x), 1 / 3);

  if (Math.abs(Q) < 1e-5) {
    const u = cbrt(-q / 2);
    const x1 = 2 * u - shift;
    const x2 = -u - shift;
    return [x1, x2];
  } else if (Q > 0) {
    const u = cbrt(-q / 2 + Math.sqrt(Q));
    const v = cbrt(-q / 2 - Math.sqrt(Q));
    const x1 = u + v - shift;
    return [x1];
  } else {
    const r = Math.sqrt(-(p ** 3) / 27);
    const t = 2 * Math.sqrt(-p / 3);
    const phi = (1 / 3) * Math.acos(-q / (2 * r));
    return [
      t * Math.cos(phi) - shift,
      t * Math.cos(phi + (2 * Math.PI) / 3) - shift,
      t * Math.cos(phi + (4 * Math.PI) / 3) - shift,
    ];
  }
}

/**
 * Метод касательных
 * @param {number[]} coeffs
 * @param {number} eps - точность
 * @param {number} N - количество итераций
 * @param {number} x0 - предполагаемый корень
 */
function tangentsMethod(coeffs, eps, N, x0) {
  for (let i = 0; i < N; i++) {
    const fx = f(coeffs, x0);
    const dfx = df(coeffs, x0);
    if (Math.abs(dfx) < 1e-14) {
      if (Math.abs(fx) < eps) return x0;
      else return null;
    }
    const delta = fx / dfx;
    x0 -= delta;
    if (Math.abs(delta) < eps) {
      if (Math.abs(f(coeffs, x0)) < eps) return x0;
    }
  }
  return null;
}

/**
 * Метод хорд
 * @param {number[]} coeffs
 * @param {number} eps - точность
 * @param {number} N - количество итераций
 * @param {number} l - левая граница
 * @param {number} r - правая граница
 */
function chordsMethod(coeffs, eps, N, l, r) {
  let x = l;
  for (let i = 0; i < N; i++) {
    let xPrev = x;
    let fl = f(coeffs, l);
    let fr = f(coeffs, r);
    if (Math.abs(fr - fl) < 1e-12) {
      if (Math.abs(f(coeffs, x)) < eps) return x;
      l += eps;
      continue;
    }
    x = l - (fl * (r - l)) / (fr - fl);
    const fx = f(coeffs, x);
    if (fx === 0) return x;
    if (fl * fx < 0) r = x;
    else l = x;
    if (Math.abs(x - xPrev) < eps) {
      if (Math.abs(fx) < eps) return x;
    }
  }
  return null;
}

/**
 * Комбинированный метод (метод хорд и касательных)
 * @param {number[]} coeffs
 * @param {number} eps - точность
 * @param {number} N - количество итераций
 * @param {number} l - левая граница
 * @param {number} r - правая граница
 */
function chordsAndTangentsMethod(coeffs, eps, N, l, r) {
  for (let i = 0; i < N; i++) {
    let fl = f(coeffs, l),
      fr = f(coeffs, r);
    if (Math.abs(fr - fl) < 1e-18) {
      if (Math.abs(f(coeffs, (l + r) / 2)) < eps) return (l + r) / 2;
      l += eps;
      continue;
    }
    if (fr * ddf(coeffs, r) > 0) {
      if (Math.abs(df(coeffs, r)) > 1e-15) r = r - fr / df(coeffs, r);
      l = l - (fl * (r - l)) / (fr - fl);
    } else {
      if (Math.abs(df(coeffs, l)) > 1e-15) l = l - fl / df(coeffs, l);
      r = r - (fr * (r - l)) / (fr - fl);
    }
    if (Math.abs(r - l) < eps) {
      if (Math.abs(f(coeffs, (l + r) / 2)) < eps) return (l + r) / 2;
    }
  }
  return null;
}

// -------------------------------------------------------------------------
//            ФУНКЦИИ ДЛЯ ВАЛИДАЦИИ ПОЛЬЗОВАТЕЛЬСКОГО ВВОДА
// -------------------------------------------------------------------------

/**
 * Проверка числа
 * @param {number} val
 */
function isValidNumber(val) {
  return isFinite(val) && Math.abs(val) <= Number.MAX_VALUE;
}

/**
 * Запрос ввода строки
 */
function readLine() {
  const buffer = Buffer.alloc(500);
  const bytesRead = fs.readSync(0, buffer, 0, 500);
  if (bytesRead === 0) return "";
  return buffer.toString("utf8", 0, bytesRead).trim();
}

/**
 * Получение данных из пользовательского ввода
 */
function getInput() {
  let coeffsInput, eps, N, x0, l, r;
  process.stdout.write(
    "Введите 7 коэффициентов полинома (a b c d n m k) через пробел:\n> ",
  );
  while (true) {
    let input = readLine();
    if (!input) continue;
    let parts = input.split(/\s+/);
    if (parts.length !== 7) {
      process.stdout.write(
        "Ошибка: Введено не 7 параметров. Повторите ввод:\n> ",
      );
      continue;
    }
    coeffsInput = parts.map(Number);
    if (coeffsInput.some((t) => !isValidNumber(t))) {
      process.stdout.write(
        "Ошибка: Не удалось прочитать числа или числа слишком большие. Повторите ввод:\n> ",
      );
      continue;
    }
    if (coeffsInput.every((t) => t === 0)) {
      console.log("Решением уравнения является любое число");
      process.exit(0);
    }
    if (
      coeffsInput.slice(0, -1).every((c) => c === 0) &&
      coeffsInput[6] !== 0
    ) {
      console.log("Решений нет");
      process.exit(0);
    }
    break;
  }

  const bound = cachyBound(coeffsInput);
  let [lx, rx] = findInterval(coeffsInput, bound);

  let start;
  if (f(coeffsInput, lx) * ddf(coeffsInput, lx) > 0) start = lx;
  else start = rx;

  process.stdout.write(
    `Введите по порядку через пробел:\n  eps - точность (1e-10)\n  N - количество итераций (1000)\n  x0 - начальное значение (0)\n  l - левая граница (${-bound})\n  r - правая граница (${bound})\nДля установки всех значений по умолчанию нажмите "Enter". Для отдельного значения по умолчанию введите символ "-" на месте нужного параметра:\n> `,
  );
  while (true) {
    let input = readLine();
    const defaultValues = [1e-10, 1000, start, -bound, bound];
    if (!input) {
      [eps, N, x0, l, r] = defaultValues;
      break;
    }
    let parts = input.split(/\s+/);
    if (parts.length !== 5) {
      process.stdout.write(
        "Ошибка: Должно быть введено ровно 5 параметров. Повторите ввод:\n> ",
      );
      continue;
    }
    if (!parts.every((val) => isValidNumber(val) || val === "-")) {
      process.stdout.write(
        "Ошибка: Введены некорректные параметры. Повторите ввод:\n> ",
      );
      continue;
    }

    [eps, N, x0, l, r] = parts.map((val, ind) =>
      val === "-" ? defaultValues[ind] : parseFloat(val),
    );
    break;
  }

  return [coeffsInput, eps, N, x0, l, r];
}

/**
 * Вывод корней без повторов
 * @param {number[]} solutions
 */
function printRoots(solutions) {
  console.log(
    "Найденные корни:",
    ...new Set(solutions.map((val) => formatRoot(val)).sort((a, b) => a - b)),
  );
}

/**
 * Округление корня
 * @param {number} x
 */
function formatRoot(x) {
  if (Math.abs(Math.round(x * 1e6) / 1e6 - Math.round(x)) < 0.1) {
    return Math.round(x);
  }
  return parseFloat(x.toFixed(3));
}

// -------------------------------------------------------------------------
//                            ГЛАВНАЯ ФУНКЦИЯ
// -------------------------------------------------------------------------
function main() {
  let solutions = [];
  let [coeffs, eps, N, x0, l, r] = getInput();

  let root;
  // ПЕРВЫЙ КОРЕНЬ (6 степень)
  if (coeffs[0] !== 0) {
    root = tangentsMethod(coeffs, eps, N, x0);

    if (root !== null) {
      solutions.push(root);
      coeffs = hornersShema(coeffs, root);
    } else {
      console.log("Корней нет");
      return;
    }
  } else {
    coeffs = coeffs.slice(1);
  }

  // ВТОРОЙ КОРЕНЬ (5 степень)
  if (coeffs[0] !== 0) {
    root = chordsMethod(coeffs, eps, N, ...findInterval(coeffs, r));
    if (root !== null) {
      solutions.push(root);
      coeffs = hornersShema(coeffs, root);
    } else {
      printRoots(solutions);
      return;
    }
  } else {
    coeffs = coeffs.slice(1);
  }

  // ТРЕТИЙ КОРЕНЬ (4 степень)
  if (coeffs[0] !== 0) {
    root = chordsAndTangentsMethod(coeffs, eps, N, l, r);
    if (root !== null) {
      solutions.push(root);
      coeffs = hornersShema(coeffs, root);
      coeffs = coeffs.map((c) => (Math.abs(c) < 1e-12 ? 0 : c));
    } else {
      printRoots(solutions);
      return;
    }
  } else {
    coeffs = coeffs.slice(1);
  }

  let roots;
  // МЕТОД КАРДАНО (3 степень)
  if (coeffs[0] !== 0) {
    roots = cardanosMethod(coeffs);
    solutions.push(...roots);
    printRoots(solutions);
    return;
  } else {
    coeffs = coeffs.slice(1);
  }

  // ТЕОРЕМА ВИЕТА (2 степень)
  if (coeffs[0] !== 0) {
    roots = vietasTheorem(coeffs);
    solutions.push(...roots);
    printRoots(solutions);
    return;
  } else {
    coeffs = coeffs.slice(1);
  }

  // ЛИНЕЙНОЕ УРАВНЕНИЕ (1 степень)
  if (coeffs[0] !== 0) {
    solutions.push(-coeffs[1] / coeffs[0]);
    printRoots(solutions);
    return;
  }
}

try {
  main();
} catch (e) {
  if (e.message.includes("EOF")) console.log("\nВыход из программы.");
  else console.error(e);
}

// main([2, 10, 12, 0, 0, 0, 0]); // -3, -2, 0
// main([3, 18, 27, 0, 0, 0, 0]); // -3, 0
// main([1, -21, 175, -735, 1624, -1764, 720]); // 1, 2, 3, 4, 5, 6
// main([1,, 0, -40, 0, 17, 0, -2]); // -1.414, 0.5 1.414
// main([1, -5, 4, 10, -11, -5, 6]); // -1, 1, 2, 3
// main([0, 1, -11, 45, -85, 74, -24]); // 1, 2, 3, 4
// main([1, 0, 0, 0, 0, 0, -1]); // -1, 1
// main([1, 0, -4, 0, -1, 0, 4]); // -2, -1, 1, 2
// main([1, -4, -6, 32, 1, -60, 36]); // -2, 1, 3,
// main([1, 0, -10, 0, 31, 0, -30]); // -2.236, -1.732, -1.414, 1.414, 1.732, 2.236
// main([5040, -8028, 5104, -1665, 295, -27, 1]); //  0.143, 0.167, 0.2, 0.25, 0.333, 0.5
// main([0.25, 0, -3.25, 0, 9, 0, 0]); // -3, -2, 0, 2, 3
// main([1, -2, -17, 68, -97, 62, -15]); // -5, 1, 3
// main([1, -3, 3, -2, 3, -3, 1]); // 1
// main([2, -1, -4, -4, 3, 12, -6]); // -1.414, 0.5, 1.414, 1.442
