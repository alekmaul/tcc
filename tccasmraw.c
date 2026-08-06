static void asmraw_instr(void)
{
    CString cstr;
    int has_quote = 0;
    int line_start = 1;

    next();
    if (tok != '(') {
        expect("(");
    }

    /* Skip whitespace after opening parenthesis */
    while (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n') {
        inp();
    }

    if (ch == '"') {
        has_quote = 1;
        inp();
    }

    cstr_new(&cstr);

    while (ch != CH_EOB) {
        /* Trim leading whitespace on new lines */
        if (line_start) {
            while (ch == ' ' || ch == '\t') {
                inp();
            }
        }

        /* Stop at closing quote or closing parenthesis */
        if (has_quote && ch == '"') {
            break;
        }
        if (!has_quote && ch == ')') {
            break;
        }

        if (ch == '\r') {
            inp();
            continue;
        }

        if (ch == '\n') {
            line_start = 1;
            cstr_ccat(&cstr, '\n');
            inp();
            continue;
        }

        line_start = 0;
        cstr_ccat(&cstr, ch);
        inp();
    }

    cstr_ccat(&cstr, '\0');

    /* Trim trailing whitespace, newlines, and leftover quotes */
    char *p = (char *) cstr.data;
    int len = cstr.size - 1;
    while (len > 0
           && (p[len - 1] == ' ' || p[len - 1] == '\t' || p[len - 1] == '\r' || p[len - 1] == '\n'
               || p[len - 1] == '"')) {
        p[--len] = '\0';
    }

    if (len > 0) {
        pr("%s\n", p);
    }

    cstr_free(&cstr);

    if (has_quote && ch == '"') {
        inp();
    }

    while (ch != ')' && ch != CH_EOB) {
        inp();
    }

    if (ch == ')') {
        inp();
    }
    next();
}