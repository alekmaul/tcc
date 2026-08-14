static void asmraw_instr(void)
{
    CString cstr;
    int has_quote = 0;
    int line_start = 1;
    int paren_depth = 0; /* tracks nested '(' in the unquoted case */

    next();
    if (tok != '(') {
        expect("(");
    }

    /* IMPORTANT: next_nomacro1() only updates file->buf_ptr internally
       (via its local cursor 'p'); it never touches the global 'ch'
       variable. Since the code below reads raw source text directly
       through ch/inp() instead of going through the normal tokenizer,
       we must manually resync ch with the real buffer position here -
       otherwise ch would still hold a stale value left over from
       whatever the tokenizer last did (e.g. scanning the __asmraw__
       identifier), causing the raw scan below to start one character
       too early. */
    ch = file->buf_ptr[0];

    /* Skip whitespace after the opening parenthesis */
    while (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n') {
        inp();
    }

    /* Optional quoted form: __asmraw__ ("...") */
    if (ch == '"') {
        has_quote = 1;
        inp();
    }

    cstr_new(&cstr);

    while (ch != CH_EOB) {
        /* Trim leading whitespace on each new line */
        if (line_start) {
            while (ch == ' ' || ch == '\t') {
                inp();
            }
        }

        /* End of quoted block */
        if (has_quote && ch == '"') {
            break;
        }
        /* Unquoted block: track parenthesis nesting so inner '(' ')'
           pairs (e.g. addressing modes like (ptr),y) don't terminate
           the block early; only an unmatched ')' ends it */
        if (!has_quote) {
            if (ch == '(') {
                paren_depth++;
            } else if (ch == ')') {
                if (paren_depth == 0) {
                    break;
                }
                paren_depth--;
            }
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

    /* Emit the collected raw assembly text as-is */
    if (len > 0) {
        pr("%s\n", p);
    }

    cstr_free(&cstr);

    /* Consume the closing quote, if any */
    if (has_quote && ch == '"') {
        inp();
    }

    /* Skip any trailing junk up to the closing ')' */
    while (ch != ')' && ch != CH_EOB) {
        inp();
    }

    if (ch == ')') {
        inp();
    }
    next();
}