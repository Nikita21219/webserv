n = int(input())
y0 = 1
y1 = 3
def trans(n, y0, y1):
    if y0 == 1:
        if y1 == 2:
            if n % 2 == 0 and n != 1:
                return trans(n - 1, 1, 3), (n, 1, 2), trans(n - 1, 3, 2)
            elif n % 2 != 0 and n != 1:
                return trans(n - 1, 1, 3), (n, 1, 2), trans(n - 1, 3, 2)
            else:
                return n, 1, 2
        if y1 == 3:
            if n % 2 == 0 and n != 1:
                return trans(n - 1, 1, 2), (n, 1, 3), trans(n - 1, 2, 3)
            elif n % 2 != 0 and n != 1:
                return trans(n - 1, 1, 2), (n, 1, 3), trans(n - 1, 2, 3)
            else:
                return n, 1, 3
    if y0 == 2:
        if y1 == 1:
            if n % 2 == 0 and n != 1:
                return trans(n - 1, 2, 3), (n, 2, 1), trans(n - 1, 3, 1)
            elif n % 2 != 0 and n != 1:
                return trans(n - 1, 2, 3), (n, 2, 1), trans(n - 1, 3, 1)
            else:
                return n, 2, 1
        if y1 == 3:
            if n % 2 == 0 and n != 1:
                return trans(n - 1, 2, 1), (n, 2, 3), trans(n - 1, 1, 3)
            elif n % 2 != 0 and n != 1:
                return trans(n - 1, 2, 1), (n, 2, 3), trans(n - 1, 1, 3)
            else:
                return n, 2, 3
    if y0 == 3:
        if y1 == 1:
            if n % 2 == 0 and n != 1:
                return trans(n - 1, 3, 2), (n, 3, 1), trans(n - 1, 2, 1)
            elif n % 2 != 0 and n != 1:
                return trans(n - 1, 3, 2), (n, 3, 1), trans(n - 1, 2, 1)
            else:
                return n, 3, 1
  